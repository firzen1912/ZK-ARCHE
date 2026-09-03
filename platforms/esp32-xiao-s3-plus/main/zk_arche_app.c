/* ZK-ARCHE client firmware for Seeed Studio XIAO ESP32-S3 Plus. */

#include "auth/iot_auth.h"
#include "auth/auth_crypto.h"
#include "auth/auth_proto.h"
#include "auth/auth_store.h"

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "nvs.h"
#include "nvs_flash.h"

#define TAG "zk-arche"
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define USER_LED_GPIO      GPIO_NUM_21
#define CRED_BLOB_LEN      146u
#define CRED_NVS_NAMESPACE "zk_arche"
#define CRED_NVS_KEY       "cred_v1"

#ifdef CONFIG_ZK_ALLOW_TOFU_SETUP
#define ZK_ALLOW_TOFU 1
#else
#define ZK_ALLOW_TOFU 0
#endif

static EventGroupHandle_t s_wifi_event_group;
static int s_wifi_retry;
static uint8_t s_out[AUTH_MAX_DATAGRAM];
static uint8_t s_in[AUTH_MAX_DATAGRAM];

static void put_u64_le(uint8_t out[8], uint64_t v)
{
    for (size_t i = 0; i < 8; ++i) out[i] = (uint8_t)(v >> (8u * i));
}

static uint64_t get_u64_le(const uint8_t in[8])
{
    uint64_t v = 0;
    for (size_t i = 0; i < 8; ++i) v |= ((uint64_t)in[i]) << (8u * i);
    return v;
}

/* Same semantic credential fields/version as c/include/auth/store.h. */
static void creds_encode(const auth_credentials_t *c, uint8_t out[CRED_BLOB_LEN])
{
    static const uint8_t magic[8] = {'I','A','C','R','E','D',0,1};
    memset(out, 0, CRED_BLOB_LEN);
    memcpy(out, magic, sizeof magic);
    out[8] = 1;
    out[9] = (uint8_t)((c->has_pinned_server ? 1u : 0u) |
                       (c->has_role ? 2u : 0u));
    size_t p = 10;
    memcpy(out + p, c->device_root, 32); p += 32;
    memcpy(out + p, c->server_pub_pinned, 32); p += 32;
    memcpy(out + p, c->role_commitment, 32); p += 32;
    memcpy(out + p, c->role_blind, 32); p += 32;
    put_u64_le(out + p, c->role_code);
}

static int creds_decode(auth_credentials_t *c, const uint8_t in[CRED_BLOB_LEN])
{
    static const uint8_t magic[8] = {'I','A','C','R','E','D',0,1};
    if (memcmp(in, magic, sizeof magic) != 0 || in[8] != 1) return -1;

    memset(c, 0, sizeof *c);
    c->has_pinned_server = (in[9] & 1u) != 0;
    c->has_role = (in[9] & 2u) != 0;
    size_t p = 10;
    memcpy(c->device_root, in + p, 32); p += 32;
    memcpy(c->server_pub_pinned, in + p, 32); p += 32;
    memcpy(c->role_commitment, in + p, 32); p += 32;
    memcpy(c->role_blind, in + p, 32); p += 32;
    c->role_code = get_u64_le(in + p);
    return 0;
}

static esp_err_t creds_load(auth_credentials_t *c, bool *found)
{
    *found = false;
    nvs_handle_t h;
    esp_err_t err = nvs_open(CRED_NVS_NAMESPACE, NVS_READONLY, &h);
    if (err == ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    if (err != ESP_OK) return err;

    uint8_t blob[CRED_BLOB_LEN];
    size_t n = sizeof blob;
    err = nvs_get_blob(h, CRED_NVS_KEY, blob, &n);
    nvs_close(h);
    if (err == ESP_ERR_NVS_NOT_FOUND) return ESP_OK;
    if (err != ESP_OK) return err;
    if (n != sizeof blob || creds_decode(c, blob) != 0) return ESP_ERR_INVALID_STATE;
    *found = true;
    return ESP_OK;
}

static esp_err_t creds_save(const auth_credentials_t *c)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(CRED_NVS_NAMESPACE, NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    uint8_t blob[CRED_BLOB_LEN];
    creds_encode(c, blob);
    err = nvs_set_blob(h, CRED_NVS_KEY, blob, sizeof blob);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

static void wifi_event_handler(void *arg, esp_event_base_t base,
                               int32_t event_id, void *event_data)
{
    (void)arg;
    if (base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_wifi_retry < 10) {
            ++s_wifi_retry;
            esp_wifi_connect();
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        const ip_event_got_ip_t *event = (const ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Wi-Fi ready: " IPSTR, IP2STR(&event->ip_info.ip));
        s_wifi_retry = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static esp_err_t wifi_connect(void)
{
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) return ESP_ERR_NO_MEM;

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wc = {0};
    strlcpy((char *)wc.sta.ssid, CONFIG_ZK_WIFI_SSID, sizeof wc.sta.ssid);
    strlcpy((char *)wc.sta.password, CONFIG_ZK_WIFI_PASSWORD, sizeof wc.sta.password);
    wc.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wc));
    ESP_ERROR_CHECK(esp_wifi_start());

    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE, pdFALSE, pdMS_TO_TICKS(30000));
    return (bits & WIFI_CONNECTED_BIT) ? ESP_OK : ESP_FAIL;
}

static int udp_open(struct sockaddr_in *server)
{
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (fd < 0) return -1;
    memset(server, 0, sizeof *server);
    server->sin_family = AF_INET;
    server->sin_port = htons((uint16_t)CONFIG_ZK_SERVER_PORT);
    if (inet_pton(AF_INET, CONFIG_ZK_SERVER_IPV4, &server->sin_addr) != 1) {
        close(fd);
        errno = EINVAL;
        return -1;
    }
    struct timeval timeout = {.tv_sec = 5, .tv_usec = 0};
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout) != 0) {
        close(fd);
        return -1;
    }
    return fd;
}

static auth_err_t udp_roundtrip(int fd, const struct sockaddr_in *server,
                                const uint8_t *out, size_t out_len,
                                uint8_t *in, size_t in_cap, size_t *in_len)
{
    for (int attempt = 0; attempt < 2; ++attempt) {
        ssize_t sent = sendto(fd, out, out_len, 0,
                              (const struct sockaddr *)server, sizeof *server);
        if (sent < 0 || (size_t)sent != out_len) return AUTH_ERR_IO;
        ssize_t got = recvfrom(fd, in, in_cap, 0, NULL, NULL);
        if (got >= 0) {
            *in_len = (size_t)got;
            return AUTH_OK;
        }
        if (errno != EAGAIN && errno != EWOULDBLOCK) return AUTH_ERR_IO;
    }
    return AUTH_ERR_TIMEOUT;
}

static auth_err_t ctx_from_creds(auth_client_ctx_t *ctx,
                                 const auth_credentials_t *creds,
                                 uint64_t fallback_role)
{
    memset(ctx, 0, sizeof *ctx);
    memcpy(ctx->device_root, creds->device_root, 32);
    auth_derive_device_id(ctx->device_id, creds->device_root);
    auth_derive_device_scalar(ctx->device_sk, creds->device_root);
    auth_err_t err = auth_scalarmult_base(ctx->device_pub, ctx->device_sk);
    if (err != AUTH_OK) return err;
    err = auth_client_ctx_init(ctx);
    if (err != AUTH_OK) return err;
    ctx->has_pinned_server = creds->has_pinned_server;
    memcpy(ctx->server_pub_pinned, creds->server_pub_pinned, 32);
    ctx->has_role = creds->has_role;
    memcpy(ctx->role_commitment, creds->role_commitment, 32);
    memcpy(ctx->role_blind, creds->role_blind, 32);
    ctx->role_code = creds->has_role ? creds->role_code : fallback_role;
    return AUTH_OK;
}

static int parse_roles(const char *s, uint64_t *out, size_t cap, size_t *n_out)
{
    *n_out = 0;
    const char *p = s;
    while (*p != '\0' && *n_out < cap) {
        char *end = NULL;
        unsigned long long v = strtoull(p, &end, 10);
        if (end == p) return -1;
        out[(*n_out)++] = (uint64_t)v;
        p = end;
        if (*p == ',') ++p;
        else if (*p != '\0') return -1;
    }
    return (*n_out > 0) ? 0 : -1;
}

static auth_err_t do_setup(int fd, const struct sockaddr_in *server,
                           auth_credentials_t *creds)
{
    auth_client_ctx_t ctx;
    auth_err_t err = ctx_from_creds(&ctx, creds, (uint64_t)CONFIG_ZK_ROLE);
    if (err != AUTH_OK) return err;

    size_t out_len = 0, in_len = 0;
    const char *token = CONFIG_ZK_PAIRING_TOKEN;
    err = auth_client_build_setup1(&ctx, (const uint8_t *)token, strlen(token),
                                   s_out, sizeof s_out, &out_len);
    if (err != AUTH_OK) return err;
    err = udp_roundtrip(fd, server, s_out, out_len, s_in, sizeof s_in, &in_len);
    if (err != AUTH_OK) return err;
    err = auth_client_handle_setup2(&ctx, s_in, in_len, ZK_ALLOW_TOFU);
    if (err != AUTH_OK) return err;

    err = auth_client_build_setup3(&ctx, s_out, sizeof s_out, &out_len);
    if (err != AUTH_OK) return err;
    err = udp_roundtrip(fd, server, s_out, out_len, s_in, sizeof s_in, &in_len);
    if (err != AUTH_OK) return err;
    err = auth_client_handle_setup_ack(&ctx, s_in, in_len);
    if (err != AUTH_OK) return err;

    creds->has_pinned_server = 1;
    memcpy(creds->server_pub_pinned, ctx.server_pub_pinned, 32);
    creds->has_role = 1;
    memcpy(creds->role_commitment, ctx.role_commitment, 32);
    memcpy(creds->role_blind, ctx.role_blind, 32);
    creds->role_code = ctx.role_code;
    return creds_save(creds) == ESP_OK ? AUTH_OK : AUTH_ERR_IO;
}

static auth_err_t do_auth(int fd, const struct sockaddr_in *server,
                          const auth_credentials_t *creds)
{
    if (!creds->has_pinned_server || !creds->has_role)
        return AUTH_ERR_CREDENTIAL_MISSING;

    auth_client_ctx_t ctx;
    auth_err_t err = ctx_from_creds(&ctx, creds, creds->role_code);
    if (err != AUTH_OK) return err;

    uint64_t allowed[AUTH_MAX_ROLES];
    size_t n_allowed = 0;
    if (parse_roles(CONFIG_ZK_ALLOWED_ROLES, allowed, AUTH_MAX_ROLES,
                    &n_allowed) != 0) return AUTH_ERR_INVALID_ARGUMENT;

    size_t out_len = 0, in_len = 0;
    err = auth_client_build_auth1(&ctx, allowed, n_allowed,
                                  s_out, sizeof s_out, &out_len);
    if (err != AUTH_OK) return err;
    err = udp_roundtrip(fd, server, s_out, out_len, s_in, sizeof s_in, &in_len);
    if (err != AUTH_OK) return err;
    err = auth_client_handle_auth2(&ctx, s_in, in_len);
    if (err != AUTH_OK) return err;

    err = auth_client_build_auth3(&ctx, s_out, sizeof s_out, &out_len);
    if (err != AUTH_OK) return err;
    err = udp_roundtrip(fd, server, s_out, out_len, s_in, sizeof s_in, &in_len);
    if (err != AUTH_OK) return err;
    return auth_client_handle_auth_ack(&ctx, s_in, in_len);
}

static void led_signal(int count)
{
    gpio_set_direction(USER_LED_GPIO, GPIO_MODE_OUTPUT);
    for (int i = 0; i < count; ++i) {
        gpio_set_level(USER_LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(120));
        gpio_set_level(USER_LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(120));
    }
}

void app_main(void)
{
    esp_err_t e = nvs_flash_init();
    if (e == ESP_ERR_NVS_NO_FREE_PAGES || e == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        e = nvs_flash_init();
    }
    ESP_ERROR_CHECK(e);

    if (auth_init() != AUTH_OK) {
        ESP_LOGE(TAG, "crypto initialization failed");
        led_signal(10);
        return;
    }

    auth_credentials_t creds;
    bool found = false;
    ESP_ERROR_CHECK(creds_load(&creds, &found));
    if (!found) {
        memset(&creds, 0, sizeof creds);
        auth_random_bytes32(creds.device_root);
        if (creds_save(&creds) != ESP_OK) {
            ESP_LOGE(TAG, "failed to persist fresh device root");
            led_signal(10);
            return;
        }
        ESP_LOGI(TAG, "created persistent device root in NVS");
    }

    uint8_t device_id[AUTH_DEVICE_ID_LEN];
    auth_derive_device_id(device_id, creds.device_root);
    ESP_LOGI(TAG, "device-id prefix %02x%02x%02x%02x; secrets are not logged",
             device_id[0], device_id[1], device_id[2], device_id[3]);

    if (strlen(CONFIG_ZK_WIFI_SSID) == 0 || wifi_connect() != ESP_OK) {
        ESP_LOGE(TAG, "Wi-Fi not configured/connected; use idf.py menuconfig");
        led_signal(10);
        return;
    }

    struct sockaddr_in server;
    int fd = udp_open(&server);
    if (fd < 0) {
        ESP_LOGE(TAG, "UDP setup failed: errno=%d", errno);
        led_signal(10);
        return;
    }

    auth_err_t err = AUTH_OK;
    if (!creds.has_pinned_server || !creds.has_role) {
        ESP_LOGI(TAG, "credentials incomplete: executing explicit SETUP");
        err = do_setup(fd, &server, &creds);
        if (err == AUTH_OK) ESP_LOGI(TAG, "SETUP complete; credentials persisted");
    }
    if (err == AUTH_OK) err = do_auth(fd, &server, &creds);
    close(fd);

    if (err != AUTH_OK) {
        ESP_LOGE(TAG, "ZK-ARCHE failed: %s", auth_strerror(err));
        led_signal(10);
        return;
    }

    ESP_LOGI(TAG, "AUTH complete under canonical ZK-ARCHE C semantics");
    led_signal(3);
}
