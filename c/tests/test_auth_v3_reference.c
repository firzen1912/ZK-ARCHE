/*
 * C conformance test for the non-advertised AUTH v3 draft primitives.
 *
 * The Rust-owned JSON fixture remains the canonical input/output corpus. This
 * test exercises the reusable C module without making protocol v3 selectable.
 */

#include "auth/auth_v3.h"

#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sodium.h>

#define JSON_VALUE_CAPACITY 129u

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

    fputs("AUTH v3 reference vector not found\n", stderr);
    abort();
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

static void load_parts(const char *json,
                       auth_v3_context_t *context,
                       auth_v3_transcript_parts_t *parts,
                       uint8_t pid[32], uint8_t a_c[32], uint8_t s_c[32],
                       uint8_t nonce_c[32], uint8_t eph_c[32],
                       uint8_t server_pub[32], uint8_t a_s[32], uint8_t s_s[32],
                       uint8_t nonce_s[32], uint8_t eph_s[32])
{
    memset(context, 0, sizeof *context);
    context->protocol_version = (uint8_t)json_u64(json, "protocol_version");
    context->suite_id = (auth_suite_t)json_u64(json, "suite_id");
    context->profile_id = (uint16_t)json_u64(json, "profile_id");
    context->selected_capabilities = (auth_caps_t)json_u64(json, "selected_capabilities");
    json_hex(json, "session_id", context->session_id, sizeof context->session_id);
    json_hex(json, "authz_context_hash", context->authz_context_hash,
             sizeof context->authz_context_hash);
    json_hex(json, "critical_extensions_hash", context->critical_extensions_hash,
             sizeof context->critical_extensions_hash);
    json_hex(json, "channel_binding_hash", context->channel_binding_hash,
             sizeof context->channel_binding_hash);

    json_hex(json, "pid", pid, 32u);
    json_hex(json, "a_c", a_c, 32u);
    json_hex(json, "s_c", s_c, 32u);
    json_hex(json, "nonce_c", nonce_c, 32u);
    json_hex(json, "eph_c", eph_c, 32u);
    json_hex(json, "server_pub", server_pub, 32u);
    json_hex(json, "a_s", a_s, 32u);
    json_hex(json, "s_s", s_s, 32u);
    json_hex(json, "nonce_s", nonce_s, 32u);
    json_hex(json, "eph_s", eph_s, 32u);

    *parts = (auth_v3_transcript_parts_t){
        .context = context,
        .pid = pid,
        .a_c = a_c,
        .s_c = s_c,
        .nonce_c = nonce_c,
        .eph_c = eph_c,
        .server_pub = server_pub,
        .a_s = a_s,
        .s_s = s_s,
        .nonce_s = nonce_s,
        .eph_s = eph_s,
    };
}

static void test_reference_vector(void)
{
    assert(auth_init() == AUTH_OK);
    char *json = load_vector_json();

    auth_v3_context_t context;
    auth_v3_transcript_parts_t parts;
    uint8_t pid[32], a_c[32], s_c[32], nonce_c[32], eph_c[32];
    uint8_t server_pub[32], a_s[32], s_s[32], nonce_s[32], eph_s[32];
    load_parts(json, &context, &parts, pid, a_c, s_c, nonce_c, eph_c,
               server_pub, a_s, s_s, nonce_s, eph_s);

    assert(context.protocol_version == AUTH_V3_DRAFT_PROTOCOL_VERSION);

    uint8_t transcript[AUTH_V3_TRANSCRIPT_MAX];
    size_t transcript_len = 0;
    assert(auth_v3_build_kc_transcript(transcript, sizeof transcript,
                                       &transcript_len, &parts) == AUTH_OK);
    assert(transcript_len == (size_t)json_u64(json, "transcript_length"));

    uint8_t th[AUTH_HASH_LEN];
    assert(auth_v3_kc_transcript_hash(th, &parts) == AUTH_OK);
    assert_json_hex_eq(json, "transcript_hash", th, sizeof th);

    uint8_t session_key[AUTH_SESSION_KEY_LEN];
    uint8_t k_s2c[AUTH_MAC_KEY_LEN];
    uint8_t k_c2s[AUTH_MAC_KEY_LEN];
    uint8_t k_complete[AUTH_MAC_KEY_LEN];
    json_hex(json, "session_key", session_key, sizeof session_key);
    assert(auth_v3_derive_kc_keys(k_s2c, k_c2s, k_complete,
                                  session_key, th) == AUTH_OK);
    assert_json_hex_eq(json, "k_s2c_v3", k_s2c, sizeof k_s2c);
    assert_json_hex_eq(json, "k_c2s_v3", k_c2s, sizeof k_c2s);
    assert_json_hex_eq(json, "k_complete_v3", k_complete, sizeof k_complete);

    static const uint8_t SERVER_FINISHED[] = "server finished v3";
    static const uint8_t CLIENT_FINISHED[] = "client finished v3";
    uint8_t tag_s[AUTH_MAC_TAG_LEN];
    uint8_t tag_c[AUTH_MAC_TAG_LEN];
    auth_v3_finished_tag(tag_s, k_s2c, SERVER_FINISHED,
                         sizeof SERVER_FINISHED - 1u, th);
    auth_v3_finished_tag(tag_c, k_c2s, CLIENT_FINISHED,
                         sizeof CLIENT_FINISHED - 1u, th);
    assert_json_hex_eq(json, "tag_s", tag_s, sizeof tag_s);
    assert_json_hex_eq(json, "tag_c", tag_c, sizeof tag_c);

    uint8_t completion_hash[AUTH_HASH_LEN];
    assert(auth_v3_completion_hash(completion_hash, th, tag_c) == AUTH_OK);
    assert_json_hex_eq(json, "completion_hash", completion_hash,
                       sizeof completion_hash);

    uint8_t tag_ack[AUTH_MAC_TAG_LEN];
    auth_v3_completion_tag(tag_ack, k_complete, completion_hash);
    assert_json_hex_eq(json, "tag_ack", tag_ack, sizeof tag_ack);

    /* Binding regression for the retained v2 same-session counterexample. */
    auth_v3_context_t changed_context = context;
    changed_context.session_id[0] ^= 0x10u;
    auth_v3_transcript_parts_t changed_parts = parts;
    changed_parts.context = &changed_context;
    uint8_t changed_th[AUTH_HASH_LEN];
    assert(auth_v3_kc_transcript_hash(changed_th, &changed_parts) == AUTH_OK);
    assert(sodium_memcmp(changed_th, th, sizeof th) != 0);

    /* Bounded API must fail closed rather than truncate a transcript. */
    size_t too_small_len = 0;
    assert(auth_v3_build_kc_transcript(transcript, transcript_len - 1u,
                                       &too_small_len, &parts) ==
           AUTH_ERR_BUFFER_TOO_SMALL);

    sodium_memzero(session_key, sizeof session_key);
    sodium_memzero(k_s2c, sizeof k_s2c);
    sodium_memzero(k_c2s, sizeof k_c2s);
    sodium_memzero(k_complete, sizeof k_complete);
    sodium_memzero(transcript, sizeof transcript);
    free(json);
}

int main(void)
{
    test_reference_vector();
    puts("AUTH v3 draft reusable primitives: ok");
    return 0;
}
