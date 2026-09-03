#include "zk_arche_stm32.h"

#include "auth/auth_crypto.h"
#include "auth/auth_proto.h"

#include <string.h>

static uint8_t s_tx[AUTH_MAX_DATAGRAM];
static uint8_t s_rx[AUTH_MAX_DATAGRAM];

static auth_err_t uart_roundtrip(UART_HandleTypeDef *uart, uint32_t timeout_ms,
                                 const uint8_t *out, size_t out_len,
                                 uint8_t *in, size_t in_cap, size_t *in_len)
{
    if (uart == NULL || out == NULL || in == NULL || in_len == NULL ||
        out_len == 0 || out_len > AUTH_MAX_DATAGRAM || out_len > 0xffffu) {
        return AUTH_ERR_INVALID_ARGUMENT;
    }

    uint8_t tx_len[2] = {
        (uint8_t)(out_len >> 8),
        (uint8_t)(out_len & 0xffu),
    };
    if (HAL_UART_Transmit(uart, tx_len, sizeof tx_len, timeout_ms) != HAL_OK ||
        HAL_UART_Transmit(uart, (uint8_t *)out, (uint16_t)out_len,
                          timeout_ms) != HAL_OK) {
        return AUTH_ERR_IO;
    }

    uint8_t rx_len[2];
    if (HAL_UART_Receive(uart, rx_len, sizeof rx_len, timeout_ms) != HAL_OK) {
        return AUTH_ERR_TIMEOUT;
    }
    size_t n = ((size_t)rx_len[0] << 8) | (size_t)rx_len[1];
    if (n == 0 || n > in_cap || n > AUTH_MAX_DATAGRAM) {
        return AUTH_ERR_PAYLOAD_TOO_LARGE;
    }
    if (HAL_UART_Receive(uart, in, (uint16_t)n, timeout_ms) != HAL_OK) {
        return AUTH_ERR_TIMEOUT;
    }
    *in_len = n;
    return AUTH_OK;
}

static auth_err_t ctx_from_creds(auth_client_ctx_t *ctx,
                                 const auth_credentials_t *creds,
                                 uint64_t fallback_role)
{
    memset(ctx, 0, sizeof *ctx);
    memcpy(ctx->device_root, creds->device_root, AUTH_DEVICE_ROOT_LEN);
    auth_derive_device_id(ctx->device_id, creds->device_root);
    auth_derive_device_scalar(ctx->device_sk, creds->device_root);

    auth_err_t err = auth_scalarmult_base(ctx->device_pub, ctx->device_sk);
    if (err != AUTH_OK) return err;
    err = auth_client_ctx_init(ctx);
    if (err != AUTH_OK) return err;

    ctx->has_pinned_server = creds->has_pinned_server;
    memcpy(ctx->server_pub_pinned, creds->server_pub_pinned, AUTH_POINT_LEN);
    ctx->has_role = creds->has_role;
    memcpy(ctx->role_commitment, creds->role_commitment, AUTH_POINT_LEN);
    memcpy(ctx->role_blind, creds->role_blind, AUTH_SCALAR_LEN);
    ctx->role_code = creds->has_role ? creds->role_code : fallback_role;
    return AUTH_OK;
}

static auth_err_t do_setup(const zk_stm32_client_config_t *cfg,
                           auth_credentials_t *creds)
{
    auth_client_ctx_t ctx;
    auth_err_t err = ctx_from_creds(&ctx, creds, cfg->requested_role);
    if (err != AUTH_OK) return err;

    size_t tx_len = 0, rx_len = 0;
    const uint8_t *token = cfg->pairing_token;
    size_t token_len = cfg->pairing_token_len;
    if (token == NULL) token_len = 0;
    if (token_len > AUTH_MAX_PAIRING_TOKEN) return AUTH_ERR_INVALID_ARGUMENT;

    err = auth_client_build_setup1(&ctx, token, token_len,
                                   s_tx, sizeof s_tx, &tx_len);
    if (err == AUTH_OK) {
        err = uart_roundtrip(cfg->uart, cfg->uart_timeout_ms,
                             s_tx, tx_len, s_rx, sizeof s_rx, &rx_len);
    }
    if (err == AUTH_OK) {
        err = auth_client_handle_setup2(&ctx, s_rx, rx_len,
                                        cfg->allow_tofu_setup);
    }
    if (err == AUTH_OK) {
        err = auth_client_build_setup3(&ctx, s_tx, sizeof s_tx, &tx_len);
    }
    if (err == AUTH_OK) {
        err = uart_roundtrip(cfg->uart, cfg->uart_timeout_ms,
                             s_tx, tx_len, s_rx, sizeof s_rx, &rx_len);
    }
    if (err == AUTH_OK) {
        err = auth_client_handle_setup_ack(&ctx, s_rx, rx_len);
    }

    if (err == AUTH_OK) {
        creds->has_pinned_server = 1;
        memcpy(creds->server_pub_pinned, ctx.server_pub_pinned, AUTH_POINT_LEN);
        creds->has_role = 1;
        memcpy(creds->role_commitment, ctx.role_commitment, AUTH_POINT_LEN);
        memcpy(creds->role_blind, ctx.role_blind, AUTH_SCALAR_LEN);
        creds->role_code = ctx.role_code;
        err = cfg->save_creds(cfg->store_ctx, creds);
    }

    auth_zeroize(&ctx, sizeof ctx);
    return err;
}

static auth_err_t do_auth(const zk_stm32_client_config_t *cfg,
                          const auth_credentials_t *creds)
{
    if (!creds->has_pinned_server || !creds->has_role) {
        return AUTH_ERR_CREDENTIAL_MISSING;
    }
    if (cfg->n_allowed_roles == 0 || cfg->n_allowed_roles > AUTH_MAX_ROLES) {
        return AUTH_ERR_INVALID_ARGUMENT;
    }

    auth_client_ctx_t ctx;
    auth_err_t err = ctx_from_creds(&ctx, creds, creds->role_code);
    if (err != AUTH_OK) return err;

    size_t tx_len = 0, rx_len = 0;
    err = auth_client_build_auth1(&ctx, cfg->allowed_roles,
                                  cfg->n_allowed_roles,
                                  s_tx, sizeof s_tx, &tx_len);
    if (err == AUTH_OK) {
        err = uart_roundtrip(cfg->uart, cfg->uart_timeout_ms,
                             s_tx, tx_len, s_rx, sizeof s_rx, &rx_len);
    }
    if (err == AUTH_OK) err = auth_client_handle_auth2(&ctx, s_rx, rx_len);
    if (err == AUTH_OK) {
        err = auth_client_build_auth3(&ctx, s_tx, sizeof s_tx, &tx_len);
    }
    if (err == AUTH_OK) {
        err = uart_roundtrip(cfg->uart, cfg->uart_timeout_ms,
                             s_tx, tx_len, s_rx, sizeof s_rx, &rx_len);
    }
    if (err == AUTH_OK) err = auth_client_handle_auth_ack(&ctx, s_rx, rx_len);

    auth_zeroize(&ctx, sizeof ctx);
    return err;
}

auth_err_t zk_stm32_client_run(const zk_stm32_client_config_t *cfg)
{
    if (cfg == NULL || cfg->uart == NULL || cfg->rng == NULL ||
        cfg->load_creds == NULL || cfg->save_creds == NULL) {
        return AUTH_ERR_INVALID_ARGUMENT;
    }

    auth_err_t err = zk_stm32_install_sodium_rng(cfg->rng);
    if (err != AUTH_OK) return err;
    err = auth_init();
    if (err != AUTH_OK) return err;

    auth_credentials_t creds;
    memset(&creds, 0, sizeof creds);
    int found = 0;
    err = cfg->load_creds(cfg->store_ctx, &creds, &found);
    if (err != AUTH_OK) return err;

    if (!found) {
        auth_random_bytes32(creds.device_root);
        err = cfg->save_creds(cfg->store_ctx, &creds);
        if (err != AUTH_OK) {
            auth_zeroize(&creds, sizeof creds);
            return err;
        }
    }

    if (!creds.has_pinned_server || !creds.has_role) {
        err = do_setup(cfg, &creds);
    }
    if (err == AUTH_OK) err = do_auth(cfg, &creds);

    auth_zeroize(&creds, sizeof creds);
    return err;
}
