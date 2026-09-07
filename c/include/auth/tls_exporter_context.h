#ifndef AUTH_TLS_EXPORTER_CONTEXT_H
#define AUTH_TLS_EXPORTER_CONTEXT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct auth_tls_exporter_context_v1 {
    const uint8_t *application_id; size_t application_id_len;
    const uint8_t *alpn; size_t alpn_len;
    const uint8_t *deployment_id; size_t deployment_id_len;
    const uint8_t *initiator_id; size_t initiator_id_len;
    const uint8_t *responder_id; size_t responder_id_len;
    uint16_t protocol_version;
    uint16_t suite_id;
    uint16_t profile_id;
    const uint8_t *auth_instance_id; size_t auth_instance_id_len;
    const uint8_t *auth_transcript_hash; size_t auth_transcript_hash_len;
} auth_tls_exporter_context_v1_t;

typedef enum auth_tls_exporter_context_result {
    AUTH_TLS_EXPORTER_CONTEXT_OK = 0,
    AUTH_TLS_EXPORTER_CONTEXT_EMPTY_REQUIRED = 1,
    AUTH_TLS_EXPORTER_CONTEXT_EMPTY_ALPN = 2,
    AUTH_TLS_EXPORTER_CONTEXT_FIELD_TOO_LONG = 3,
    AUTH_TLS_EXPORTER_CONTEXT_INVALID_ARGUMENT = 4
} auth_tls_exporter_context_result_t;

/* Construct only the canonical 32-byte SHA-256 exporter context. This does
 * not execute TLS or confer ZK-ARCHE identity, trust, or authorization. */
auth_tls_exporter_context_result_t auth_tls_exporter_context_v1_digest(
    uint8_t out[32],
    const auth_tls_exporter_context_v1_t *ctx,
    bool allow_empty_alpn);

#ifdef __cplusplus
}
#endif

#endif /* AUTH_TLS_EXPORTER_CONTEXT_H */
