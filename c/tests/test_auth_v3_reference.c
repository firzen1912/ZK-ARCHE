/*
 * Independent C reproduction of the non-advertised AUTH v3 draft vector.
 *
 * The Rust-owned JSON fixture is the canonical input/output corpus for this
 * experiment. This test deliberately keeps the construction local to the test
 * binary: passing it demonstrates byte-level Rust/C reproducibility, but does
 * not make AUTH v3 normative, selectable, or part of production dispatch.
 */

#include "auth/auth_crypto.h"

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sodium.h>

#define TRANSCRIPT_CAPACITY 768u
#define JSON_VALUE_CAPACITY 129u

static const uint8_t KC_V3_DOMAIN[] = "zk-arche/kc/v3";

static char *load_vector_json(void)
{
    static const char *paths[] = {
        "../rust/test-vectors/auth-v3/reference-primitives-v1.json",
        "rust/test-vectors/auth-v3/reference-primitives-v1.json",
    };

    for (size_t i = 0; i < sizeof paths / sizeof paths[0]; ++i) {
        FILE *fp = fopen(paths[i], "rb");
        if (fp == NULL) continue;
        assert(fseek(fp, 0, SEEK_END) == 0);
        long end = ftell(fp);
        assert(end > 0);
        assert(fseek(fp, 0, SEEK_SET) == 0);

        size_t len = (size_t)end;
        char *buf = (char *)malloc(len + 1u);
        assert(buf != NULL);
        assert(fread(buf, 1u, len, fp) == len);
        assert(fclose(fp) == 0);
        buf[len] = '\0';
        return buf;
    }

    assert(!"AUTH v3 reference vector not found");
    return NULL;
}

static const char *json_value_start(const char *json, const char *key)
{
    char needle[96];
    int n = snprintf(needle, sizeof needle, "\"%s\"", key);
    assert(n > 0 && (size_t)n < sizeof needle);

    const char *p = strstr(json, needle);
    assert(p != NULL);
    p += (size_t)n;
    while (*p != '\0' && isspace((unsigned char)*p)) ++p;
    assert(*p == ':');
    ++p;
    while (*p != '\0' && isspace((unsigned char)*p)) ++p;
    return p;
}

static void json_string(const char *json, const char *key,
                        char out[JSON_VALUE_CAPACITY])
{
    const char *p = json_value_start(json, key);
    assert(*p == '"');
    ++p;
    const char *end = strchr(p, '"');
    assert(end != NULL);
    size_t len = (size_t)(end - p);
    assert(len + 1u <= JSON_VALUE_CAPACITY);
    memcpy(out, p, len);
    out[len] = '\0';
}

static uint64_t json_u64(const char *json, const char *key)
{
    const char *p = json_value_start(json, key);
    char *end = NULL;
    unsigned long long value = strtoull(p, &end, 10);
    assert(end != p);
    return (uint64_t)value;
}

static void json_hex(const char *json, const char *key,
                     uint8_t *out, size_t out_len)
{
    char value[JSON_VALUE_CAPACITY];
    size_t decoded_len = 0;
    json_string(json, key, value);
    assert(strlen(value) == out_len * 2u);
    assert(sodium_hex2bin(out, out_len, value, strlen(value), NULL,
                          &decoded_len, NULL) == 0);
    assert(decoded_len == out_len);
}

static void assert_json_hex_eq(const char *json, const char *key,
                               const uint8_t *actual, size_t len)
{
    uint8_t expected[64];
    assert(len <= sizeof expected);
    json_hex(json, key, expected, len);
    assert(sodium_memcmp(actual, expected, len) == 0);
}

static void put_u16_le(uint8_t out[2], uint16_t value)
{
    out[0] = (uint8_t)value;
    out[1] = (uint8_t)(value >> 8);
}

static void put_u64_le(uint8_t out[8], uint64_t value)
{
    for (size_t i = 0; i < 8u; ++i) {
        out[i] = (uint8_t)(value >> (8u * i));
    }
}

static void append_bytes(uint8_t out[TRANSCRIPT_CAPACITY], size_t *offset,
                         const uint8_t *value, size_t value_len)
{
    assert(*offset <= TRANSCRIPT_CAPACITY);
    assert(value_len <= TRANSCRIPT_CAPACITY - *offset);
    memcpy(out + *offset, value, value_len);
    *offset += value_len;
}

static void append_field(uint8_t out[TRANSCRIPT_CAPACITY], size_t *offset,
                         const char *label, const uint8_t *value,
                         size_t value_len)
{
    size_t label_len = strlen(label);
    assert(label_len <= UINT8_MAX);
    assert(value_len <= UINT32_MAX);

    uint8_t label_len_u8 = (uint8_t)label_len;
    uint32_t value_len_u32 = (uint32_t)value_len;
    uint8_t value_len_le[4] = {
        (uint8_t)value_len_u32,
        (uint8_t)(value_len_u32 >> 8),
        (uint8_t)(value_len_u32 >> 16),
        (uint8_t)(value_len_u32 >> 24),
    };

    append_bytes(out, offset, &label_len_u8, 1u);
    append_bytes(out, offset, (const uint8_t *)label, label_len);
    append_bytes(out, offset, value_len_le, sizeof value_len_le);
    append_bytes(out, offset, value, value_len);
}

static size_t build_transcript(const char *json,
                               uint8_t out[TRANSCRIPT_CAPACITY])
{
    uint8_t protocol_version = (uint8_t)json_u64(json, "protocol_version");
    uint16_t suite_id = (uint16_t)json_u64(json, "suite_id");
    uint16_t profile_id = (uint16_t)json_u64(json, "profile_id");
    uint64_t selected_capabilities = json_u64(json, "selected_capabilities");
    uint8_t suite_id_le[2];
    uint8_t profile_id_le[2];
    uint8_t capabilities_le[8];

    uint8_t session_id[16];
    uint8_t authz_context_hash[32];
    uint8_t critical_extensions_hash[32];
    uint8_t channel_binding_hash[32];
    uint8_t pid[32];
    uint8_t a_c[32];
    uint8_t s_c[32];
    uint8_t nonce_c[32];
    uint8_t eph_c[32];
    uint8_t server_pub[32];
    uint8_t a_s[32];
    uint8_t s_s[32];
    uint8_t nonce_s[32];
    uint8_t eph_s[32];

    put_u16_le(suite_id_le, suite_id);
    put_u16_le(profile_id_le, profile_id);
    put_u64_le(capabilities_le, selected_capabilities);

    json_hex(json, "session_id", session_id, sizeof session_id);
    json_hex(json, "authz_context_hash", authz_context_hash,
             sizeof authz_context_hash);
    json_hex(json, "critical_extensions_hash", critical_extensions_hash,
             sizeof critical_extensions_hash);
    json_hex(json, "channel_binding_hash", channel_binding_hash,
             sizeof channel_binding_hash);
    json_hex(json, "pid", pid, sizeof pid);
    json_hex(json, "a_c", a_c, sizeof a_c);
    json_hex(json, "s_c", s_c, sizeof s_c);
    json_hex(json, "nonce_c", nonce_c, sizeof nonce_c);
    json_hex(json, "eph_c", eph_c, sizeof eph_c);
    json_hex(json, "server_pub", server_pub, sizeof server_pub);
    json_hex(json, "a_s", a_s, sizeof a_s);
    json_hex(json, "s_s", s_s, sizeof s_s);
    json_hex(json, "nonce_s", nonce_s, sizeof nonce_s);
    json_hex(json, "eph_s", eph_s, sizeof eph_s);

    size_t offset = 0;
    uint8_t domain_len = (uint8_t)(sizeof KC_V3_DOMAIN - 1u);
    append_bytes(out, &offset, &domain_len, 1u);
    append_bytes(out, &offset, KC_V3_DOMAIN, sizeof KC_V3_DOMAIN - 1u);

    append_field(out, &offset, "protocol_version", &protocol_version, 1u);
    append_field(out, &offset, "suite_id", suite_id_le, sizeof suite_id_le);
    append_field(out, &offset, "profile_id", profile_id_le,
                 sizeof profile_id_le);
    append_field(out, &offset, "selected_capabilities", capabilities_le,
                 sizeof capabilities_le);
    append_field(out, &offset, "session_id", session_id, sizeof session_id);
    append_field(out, &offset, "authz_context_hash", authz_context_hash,
                 sizeof authz_context_hash);
    append_field(out, &offset, "critical_extensions_hash",
                 critical_extensions_hash, sizeof critical_extensions_hash);
    append_field(out, &offset, "channel_binding_hash", channel_binding_hash,
                 sizeof channel_binding_hash);
    append_field(out, &offset, "pid", pid, sizeof pid);
    append_field(out, &offset, "a_c", a_c, sizeof a_c);
    append_field(out, &offset, "s_c", s_c, sizeof s_c);
    append_field(out, &offset, "nonce_c", nonce_c, sizeof nonce_c);
    append_field(out, &offset, "eph_c", eph_c, sizeof eph_c);
    append_field(out, &offset, "server_pub", server_pub, sizeof server_pub);
    append_field(out, &offset, "a_s", a_s, sizeof a_s);
    append_field(out, &offset, "s_s", s_s, sizeof s_s);
    append_field(out, &offset, "nonce_s", nonce_s, sizeof nonce_s);
    append_field(out, &offset, "eph_s", eph_s, sizeof eph_s);
    return offset;
}

static void test_reference_vector(void)
{
    assert(auth_init() == AUTH_OK);
    char *json = load_vector_json();

    uint8_t transcript[TRANSCRIPT_CAPACITY];
    size_t transcript_len = build_transcript(json, transcript);
    assert(transcript_len == (size_t)json_u64(json, "transcript_length"));

    uint8_t th[32];
    assert(crypto_hash_sha256(th, transcript,
                              (unsigned long long)transcript_len) == 0);
    assert_json_hex_eq(json, "transcript_hash", th, sizeof th);

    uint8_t session_key[32];
    uint8_t k_s2c[32];
    uint8_t k_c2s[32];
    uint8_t k_complete[32];
    json_hex(json, "session_key", session_key, sizeof session_key);

    static const uint8_t INFO_S2C[] = "kc s2c v3";
    static const uint8_t INFO_C2S[] = "kc c2s v3";
    static const uint8_t INFO_COMPLETE[] = "kc complete s2c v3";
    assert(auth_hkdf_sha256(k_s2c, sizeof k_s2c, th, sizeof th,
                            session_key, sizeof session_key,
                            INFO_S2C, sizeof INFO_S2C - 1u) == AUTH_OK);
    assert(auth_hkdf_sha256(k_c2s, sizeof k_c2s, th, sizeof th,
                            session_key, sizeof session_key,
                            INFO_C2S, sizeof INFO_C2S - 1u) == AUTH_OK);
    assert(auth_hkdf_sha256(k_complete, sizeof k_complete, th, sizeof th,
                            session_key, sizeof session_key,
                            INFO_COMPLETE, sizeof INFO_COMPLETE - 1u) == AUTH_OK);
    assert_json_hex_eq(json, "k_s2c_v3", k_s2c, sizeof k_s2c);
    assert_json_hex_eq(json, "k_c2s_v3", k_c2s, sizeof k_c2s);
    assert_json_hex_eq(json, "k_complete_v3", k_complete,
                       sizeof k_complete);

    static const uint8_t SERVER_FINISHED[] = "server finished v3";
    static const uint8_t CLIENT_FINISHED[] = "client finished v3";
    uint8_t tag_s[32];
    uint8_t tag_c[32];
    auth_hmac_tag(tag_s, k_s2c, SERVER_FINISHED,
                  sizeof SERVER_FINISHED - 1u, th);
    auth_hmac_tag(tag_c, k_c2s, CLIENT_FINISHED,
                  sizeof CLIENT_FINISHED - 1u, th);
    assert_json_hex_eq(json, "tag_s", tag_s, sizeof tag_s);
    assert_json_hex_eq(json, "tag_c", tag_c, sizeof tag_c);

    static const uint8_t COMPLETE_DOMAIN[] = "zk-arche/auth-complete/v3";
    crypto_hash_sha256_state hash_state;
    uint8_t completion_hash[32];
    assert(crypto_hash_sha256_init(&hash_state) == 0);
    assert(crypto_hash_sha256_update(&hash_state, COMPLETE_DOMAIN,
                                     sizeof COMPLETE_DOMAIN - 1u) == 0);
    assert(crypto_hash_sha256_update(&hash_state, th, sizeof th) == 0);
    assert(crypto_hash_sha256_update(&hash_state, tag_c,
                                     sizeof tag_c) == 0);
    assert(crypto_hash_sha256_final(&hash_state, completion_hash) == 0);
    assert_json_hex_eq(json, "completion_hash", completion_hash,
                       sizeof completion_hash);

    static const uint8_t SERVER_COMPLETE[] = "server complete v3";
    uint8_t tag_ack[32];
    auth_hmac_tag(tag_ack, k_complete, SERVER_COMPLETE,
                  sizeof SERVER_COMPLETE - 1u, completion_hash);
    assert_json_hex_eq(json, "tag_ack", tag_ack, sizeof tag_ack);

    /* The Rust test binds session_id; independently verify that the C byte
     * construction does too before any production v3 code is allowed. */
    uint8_t changed[TRANSCRIPT_CAPACITY];
    memcpy(changed, transcript, transcript_len);
    const uint8_t original_session[16] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
    };
    const uint8_t changed_session[16] = {
        0x10, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
    };
    uint8_t *session_pos = NULL;
    for (size_t i = 0; i + sizeof original_session <= transcript_len; ++i) {
        if (memcmp(changed + i, original_session, sizeof original_session) == 0) {
            session_pos = changed + i;
            break;
        }
    }
    assert(session_pos != NULL);
    memcpy(session_pos, changed_session, sizeof changed_session);
    uint8_t changed_th[32];
    assert(crypto_hash_sha256(changed_th, changed,
                              (unsigned long long)transcript_len) == 0);
    assert(sodium_memcmp(changed_th, th, sizeof th) != 0);

    sodium_memzero(session_key, sizeof session_key);
    sodium_memzero(k_s2c, sizeof k_s2c);
    sodium_memzero(k_c2s, sizeof k_c2s);
    sodium_memzero(k_complete, sizeof k_complete);
    free(json);
}

int main(void)
{
    test_reference_vector();
    puts("AUTH v3 draft reference vector: ok");
    return 0;
}
