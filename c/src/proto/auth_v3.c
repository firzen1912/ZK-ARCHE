/*
 * Draft AUTH v3 reference primitives from ADR 0001.
 *
 * This module is intentionally not connected to HELLO negotiation, packet
 * dispatch, or the production v2 state machine. It exists for deterministic
 * vectors, cross-language review, and later versioned promotion.
 */

#include "auth/auth_v3.h"

#include <limits.h>
#include <string.h>

#include <sodium.h>

static const uint8_t KC_V3_DOMAIN[] = "zk-arche/kc/v3";
static const uint8_t COMPLETE_DOMAIN[] = "zk-arche/auth-complete/v3";
static const uint8_t INFO_S2C[] = "kc s2c v3";
static const uint8_t INFO_C2S[] = "kc c2s v3";
static const uint8_t INFO_COMPLETE[] = "kc complete s2c v3";
static const uint8_t SERVER_COMPLETE[] = "server complete v3";

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

static auth_err_t append_bytes(uint8_t *out, size_t out_cap, size_t *offset,
                               const uint8_t *value, size_t value_len)
{
    if (out == NULL || offset == NULL || value == NULL) {
        return AUTH_ERR_INVALID_ARGUMENT;
    }
    if (*offset > out_cap || value_len > out_cap - *offset) {
        return AUTH_ERR_BUFFER_TOO_SMALL;
    }
    memcpy(out + *offset, value, value_len);
    *offset += value_len;
    return AUTH_OK;
}

static auth_err_t append_field(uint8_t *out, size_t out_cap, size_t *offset,
                               const char *label, const uint8_t *value,
                               size_t value_len)
{
    if (label == NULL || value == NULL) return AUTH_ERR_INVALID_ARGUMENT;

    size_t label_len = strlen(label);
    if (label_len > UINT8_MAX || value_len > UINT32_MAX) {
        return AUTH_ERR_INVALID_ARGUMENT;
    }

    uint8_t label_len_u8 = (uint8_t)label_len;
    uint32_t value_len_u32 = (uint32_t)value_len;
    uint8_t value_len_le[4] = {
        (uint8_t)value_len_u32,
        (uint8_t)(value_len_u32 >> 8),
        (uint8_t)(value_len_u32 >> 16),
        (uint8_t)(value_len_u32 >> 24),
    };

    auth_err_t err = append_bytes(out, out_cap, offset, &label_len_u8, 1u);
    if (err != AUTH_OK) return err;
    err = append_bytes(out, out_cap, offset, (const uint8_t *)label, label_len);
    if (err != AUTH_OK) return err;
    err = append_bytes(out, out_cap, offset, value_len_le, sizeof value_len_le);
    if (err != AUTH_OK) return err;
    return append_bytes(out, out_cap, offset, value, value_len);
}

static int parts_valid(const auth_v3_transcript_parts_t *parts)
{
    return parts != NULL && parts->context != NULL && parts->pid != NULL &&
           parts->a_c != NULL && parts->s_c != NULL && parts->nonce_c != NULL &&
           parts->eph_c != NULL && parts->server_pub != NULL &&
           parts->a_s != NULL && parts->s_s != NULL && parts->nonce_s != NULL &&
           parts->eph_s != NULL;
}

auth_err_t auth_v3_build_kc_transcript(
    uint8_t *out, size_t out_cap, size_t *out_len,
    const auth_v3_transcript_parts_t *parts)
{
    if (out == NULL || out_len == NULL || !parts_valid(parts)) {
        return AUTH_ERR_INVALID_ARGUMENT;
    }
    if (sizeof KC_V3_DOMAIN - 1u > UINT8_MAX) {
        return AUTH_ERR_INVALID_ARGUMENT;
    }

    const auth_v3_context_t *c = parts->context;
    uint8_t suite_id_le[2];
    uint8_t profile_id_le[2];
    uint8_t capabilities_le[8];
    put_u16_le(suite_id_le, c->suite_id);
    put_u16_le(profile_id_le, c->profile_id);
    put_u64_le(capabilities_le, c->selected_capabilities);

    size_t offset = 0;
    uint8_t domain_len = (uint8_t)(sizeof KC_V3_DOMAIN - 1u);
    auth_err_t err = append_bytes(out, out_cap, &offset, &domain_len, 1u);
    if (err != AUTH_OK) return err;
    err = append_bytes(out, out_cap, &offset, KC_V3_DOMAIN,
                       sizeof KC_V3_DOMAIN - 1u);
    if (err != AUTH_OK) return err;

#define APPEND_FIELD(label_, value_, len_)                                      \
    do {                                                                        \
        err = append_field(out, out_cap, &offset, (label_), (value_), (len_));  \
        if (err != AUTH_OK) return err;                                         \
    } while (0)

    APPEND_FIELD("protocol_version", &c->protocol_version, 1u);
    APPEND_FIELD("suite_id", suite_id_le, sizeof suite_id_le);
    APPEND_FIELD("profile_id", profile_id_le, sizeof profile_id_le);
    APPEND_FIELD("selected_capabilities", capabilities_le, sizeof capabilities_le);
    APPEND_FIELD("session_id", c->session_id, sizeof c->session_id);
    APPEND_FIELD("authz_context_hash", c->authz_context_hash,
                 sizeof c->authz_context_hash);
    APPEND_FIELD("critical_extensions_hash", c->critical_extensions_hash,
                 sizeof c->critical_extensions_hash);
    APPEND_FIELD("channel_binding_hash", c->channel_binding_hash,
                 sizeof c->channel_binding_hash);
    APPEND_FIELD("pid", parts->pid, AUTH_DEVICE_ID_LEN);
    APPEND_FIELD("a_c", parts->a_c, AUTH_POINT_LEN);
    APPEND_FIELD("s_c", parts->s_c, AUTH_SCALAR_LEN);
    APPEND_FIELD("nonce_c", parts->nonce_c, AUTH_NONCE_LEN);
    APPEND_FIELD("eph_c", parts->eph_c, AUTH_POINT_LEN);
    APPEND_FIELD("server_pub", parts->server_pub, AUTH_POINT_LEN);
    APPEND_FIELD("a_s", parts->a_s, AUTH_POINT_LEN);
    APPEND_FIELD("s_s", parts->s_s, AUTH_SCALAR_LEN);
    APPEND_FIELD("nonce_s", parts->nonce_s, AUTH_NONCE_LEN);
    APPEND_FIELD("eph_s", parts->eph_s, AUTH_POINT_LEN);

#undef APPEND_FIELD

    *out_len = offset;
    return AUTH_OK;
}

auth_err_t auth_v3_kc_transcript_hash(
    uint8_t th_out[AUTH_HASH_LEN],
    const auth_v3_transcript_parts_t *parts)
{
    if (th_out == NULL || !parts_valid(parts)) return AUTH_ERR_INVALID_ARGUMENT;

    uint8_t transcript[AUTH_V3_TRANSCRIPT_MAX];
    size_t transcript_len = 0;
    auth_err_t err = auth_v3_build_kc_transcript(
        transcript, sizeof transcript, &transcript_len, parts);
    if (err != AUTH_OK) return err;

    if (crypto_hash_sha256(th_out, transcript,
                           (unsigned long long)transcript_len) != 0) {
        return AUTH_ERR_UNSPECIFIED;
    }
    sodium_memzero(transcript, sizeof transcript);
    return AUTH_OK;
}

auth_err_t auth_v3_derive_kc_keys(
    uint8_t k_s2c_out[AUTH_MAC_KEY_LEN],
    uint8_t k_c2s_out[AUTH_MAC_KEY_LEN],
    uint8_t k_complete_out[AUTH_MAC_KEY_LEN],
    const uint8_t session_key[AUTH_SESSION_KEY_LEN],
    const uint8_t th_v3[AUTH_HASH_LEN])
{
    if (k_s2c_out == NULL || k_c2s_out == NULL || k_complete_out == NULL ||
        session_key == NULL || th_v3 == NULL) {
        return AUTH_ERR_INVALID_ARGUMENT;
    }

    auth_err_t err = auth_hkdf_sha256(
        k_s2c_out, AUTH_MAC_KEY_LEN, th_v3, AUTH_HASH_LEN,
        session_key, AUTH_SESSION_KEY_LEN, INFO_S2C, sizeof INFO_S2C - 1u);
    if (err != AUTH_OK) return err;
    err = auth_hkdf_sha256(
        k_c2s_out, AUTH_MAC_KEY_LEN, th_v3, AUTH_HASH_LEN,
        session_key, AUTH_SESSION_KEY_LEN, INFO_C2S, sizeof INFO_C2S - 1u);
    if (err != AUTH_OK) return err;
    return auth_hkdf_sha256(
        k_complete_out, AUTH_MAC_KEY_LEN, th_v3, AUTH_HASH_LEN,
        session_key, AUTH_SESSION_KEY_LEN,
        INFO_COMPLETE, sizeof INFO_COMPLETE - 1u);
}

void auth_v3_finished_tag(
    uint8_t tag_out[AUTH_MAC_TAG_LEN],
    const uint8_t key[AUTH_MAC_KEY_LEN],
    const uint8_t *label, size_t label_len,
    const uint8_t th_v3[AUTH_HASH_LEN])
{
    auth_hmac_tag(tag_out, key, label, label_len, th_v3);
}

auth_err_t auth_v3_completion_hash(
    uint8_t out[AUTH_HASH_LEN],
    const uint8_t th_v3[AUTH_HASH_LEN],
    const uint8_t tag_c[AUTH_MAC_TAG_LEN])
{
    if (out == NULL || th_v3 == NULL || tag_c == NULL) {
        return AUTH_ERR_INVALID_ARGUMENT;
    }

    crypto_hash_sha256_state state;
    if (crypto_hash_sha256_init(&state) != 0 ||
        crypto_hash_sha256_update(&state, COMPLETE_DOMAIN,
                                  sizeof COMPLETE_DOMAIN - 1u) != 0 ||
        crypto_hash_sha256_update(&state, th_v3, AUTH_HASH_LEN) != 0 ||
        crypto_hash_sha256_update(&state, tag_c, AUTH_MAC_TAG_LEN) != 0 ||
        crypto_hash_sha256_final(&state, out) != 0) {
        return AUTH_ERR_UNSPECIFIED;
    }
    return AUTH_OK;
}

void auth_v3_completion_tag(
    uint8_t out[AUTH_MAC_TAG_LEN],
    const uint8_t k_complete[AUTH_MAC_KEY_LEN],
    const uint8_t completion_hash[AUTH_HASH_LEN])
{
    auth_hmac_tag(out, k_complete, SERVER_COMPLETE,
                  sizeof SERVER_COMPLETE - 1u, completion_hash);
}
