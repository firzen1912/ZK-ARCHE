/*
 * auth_v3.h — draft, non-advertised AUTH v3 reference primitives.
 *
 * These helpers implement the candidate transcript/key-confirmation/completion
 * construction from ADR 0001. They are review/vector infrastructure only:
 * protocol v3 is not selectable and production v2 behavior is unchanged.
 *
 * All storage is caller-owned and bounded. No function allocates.
 */

#ifndef AUTH_V3_H
#define AUTH_V3_H

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUTH_V3_DRAFT_PROTOCOL_VERSION ((uint8_t)0x03)
#define AUTH_V3_TRANSCRIPT_MAX 768u

typedef struct auth_v3_context {
    uint8_t protocol_version;
    auth_suite_t suite_id;
    uint16_t profile_id;
    auth_caps_t selected_capabilities;
    uint8_t session_id[AUTH_SESSION_ID_LEN];
    uint8_t authz_context_hash[AUTH_HASH_LEN];
    uint8_t critical_extensions_hash[AUTH_HASH_LEN];
    uint8_t channel_binding_hash[AUTH_HASH_LEN];
} auth_v3_context_t;

typedef struct auth_v3_transcript_parts {
    const auth_v3_context_t *context;
    const uint8_t *pid;        /* AUTH_DEVICE_ID_LEN */
    const uint8_t *a_c;        /* AUTH_POINT_LEN */
    const uint8_t *s_c;        /* AUTH_SCALAR_LEN */
    const uint8_t *nonce_c;    /* AUTH_NONCE_LEN */
    const uint8_t *eph_c;      /* AUTH_POINT_LEN */
    const uint8_t *server_pub; /* AUTH_POINT_LEN */
    const uint8_t *a_s;        /* AUTH_POINT_LEN */
    const uint8_t *s_s;        /* AUTH_SCALAR_LEN */
    const uint8_t *nonce_s;    /* AUTH_NONCE_LEN */
    const uint8_t *eph_s;      /* AUTH_POINT_LEN */
} auth_v3_transcript_parts_t;

/* Build the exact candidate KC-TRANSCRIPT-v3 byte sequence. */
auth_err_t auth_v3_build_kc_transcript(
    uint8_t *out, size_t out_cap, size_t *out_len,
    const auth_v3_transcript_parts_t *parts);

/* SHA-256(KC-TRANSCRIPT-v3). */
auth_err_t auth_v3_kc_transcript_hash(
    uint8_t th_out[AUTH_HASH_LEN],
    const auth_v3_transcript_parts_t *parts);

/* Purpose-separated candidate v3 key-confirmation/completion keys. */
auth_err_t auth_v3_derive_kc_keys(
    uint8_t k_s2c_out[AUTH_MAC_KEY_LEN],
    uint8_t k_c2s_out[AUTH_MAC_KEY_LEN],
    uint8_t k_complete_out[AUTH_MAC_KEY_LEN],
    const uint8_t session_key[AUTH_SESSION_KEY_LEN],
    const uint8_t th_v3[AUTH_HASH_LEN]);

/* Directional Finished tags. Label is supplied explicitly for vector/review use. */
void auth_v3_finished_tag(
    uint8_t tag_out[AUTH_MAC_TAG_LEN],
    const uint8_t key[AUTH_MAC_KEY_LEN],
    const uint8_t *label, size_t label_len,
    const uint8_t th_v3[AUTH_HASH_LEN]);

/* SHA-256("zk-arche/auth-complete/v3" || TH_v3 || tag_c). */
auth_err_t auth_v3_completion_hash(
    uint8_t out[AUTH_HASH_LEN],
    const uint8_t th_v3[AUTH_HASH_LEN],
    const uint8_t tag_c[AUTH_MAC_TAG_LEN]);

/* HMAC(k_complete, "server complete v3" || completion_hash). */
void auth_v3_completion_tag(
    uint8_t out[AUTH_MAC_TAG_LEN],
    const uint8_t k_complete[AUTH_MAC_KEY_LEN],
    const uint8_t completion_hash[AUTH_HASH_LEN]);

#ifdef __cplusplus
}
#endif
#endif /* AUTH_V3_H */
