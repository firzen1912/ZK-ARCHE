#ifndef AUTH_TLS_EXPORTER_CONTEXT_H
#define AUTH_TLS_EXPORTER_CONTEXT_H

#include <stddef.h>
#include <stdint.h>

#define AUTH_TLS_EXPORTER_CONTEXT_LEN 32u

typedef struct {
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
} auth_tls_exporter_context_t;

typedef enum {
    AUTH_EXPORTER_CONTEXT_OK = 0,
    AUTH_EXPORTER_CONTEXT_EMPTY_REQUIRED_FIELD = -1,
    AUTH_EXPORTER_CONTEXT_EMPTY_ALPN_NOT_PERMITTED = -2,
    AUTH_EXPORTER_CONTEXT_FIELD_TOO_LONG = -3
} auth_tls_exporter_context_err_t;

auth_tls_exporter_context_err_t auth_tls_exporter_context_digest(
    const auth_tls_exporter_context_t *ctx,
    int allow_empty_alpn,
    uint8_t out[AUTH_TLS_EXPORTER_CONTEXT_LEN]);

#endif
