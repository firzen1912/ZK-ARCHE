#include "auth/tls_exporter_context.h"

#include <sodium.h>

static const uint8_t DOMAIN[] = "ZKARCHE-EXPORTER-CONTEXT-v1";

static int required_invalid(const uint8_t *p, size_t len) {
    return p == NULL || len == 0u;
}

static auth_tls_exporter_context_result_t update_len_field(
    crypto_hash_sha256_state *state,
    const uint8_t *value,
    size_t len)
{
    uint8_t be[2];
    if (len > UINT16_MAX) {
        return AUTH_TLS_EXPORTER_CONTEXT_FIELD_TOO_LONG;
    }
    if (len != 0u && value == NULL) {
        return AUTH_TLS_EXPORTER_CONTEXT_INVALID_ARGUMENT;
    }
    be[0] = (uint8_t)((len >> 8) & 0xffu);
    be[1] = (uint8_t)(len & 0xffu);
    (void)crypto_hash_sha256_update(state, be, sizeof be);
    if (len != 0u) {
        (void)crypto_hash_sha256_update(state, value, (unsigned long long)len);
    }
    return AUTH_TLS_EXPORTER_CONTEXT_OK;
}

static void update_u16(crypto_hash_sha256_state *state, uint16_t value) {
    uint8_t be[2];
    be[0] = (uint8_t)(value >> 8);
    be[1] = (uint8_t)(value & 0xffu);
    (void)crypto_hash_sha256_update(state, be, sizeof be);
}

auth_tls_exporter_context_result_t auth_tls_exporter_context_v1_digest(
    uint8_t out[32],
    const auth_tls_exporter_context_v1_t *ctx,
    bool allow_empty_alpn)
{
    crypto_hash_sha256_state state;
    auth_tls_exporter_context_result_t rc;

    if (out == NULL || ctx == NULL) {
        return AUTH_TLS_EXPORTER_CONTEXT_INVALID_ARGUMENT;
    }
    if (required_invalid(ctx->application_id, ctx->application_id_len)
        || required_invalid(ctx->deployment_id, ctx->deployment_id_len)
        || required_invalid(ctx->initiator_id, ctx->initiator_id_len)
        || required_invalid(ctx->responder_id, ctx->responder_id_len)
        || required_invalid(ctx->auth_instance_id, ctx->auth_instance_id_len)
        || required_invalid(ctx->auth_transcript_hash, ctx->auth_transcript_hash_len)) {
        return AUTH_TLS_EXPORTER_CONTEXT_EMPTY_REQUIRED;
    }
    if (ctx->alpn_len == 0u && !allow_empty_alpn) {
        return AUTH_TLS_EXPORTER_CONTEXT_EMPTY_ALPN;
    }
    if (ctx->alpn_len != 0u && ctx->alpn == NULL) {
        return AUTH_TLS_EXPORTER_CONTEXT_INVALID_ARGUMENT;
    }

    (void)crypto_hash_sha256_init(&state);
    (void)crypto_hash_sha256_update(&state, DOMAIN, sizeof DOMAIN - 1u);

#define UPDATE_FIELD(ptr, len) do { \
    rc = update_len_field(&state, (ptr), (len)); \
    if (rc != AUTH_TLS_EXPORTER_CONTEXT_OK) return rc; \
} while (0)

    UPDATE_FIELD(ctx->application_id, ctx->application_id_len);
    UPDATE_FIELD(ctx->alpn, ctx->alpn_len);
    UPDATE_FIELD(ctx->deployment_id, ctx->deployment_id_len);
    UPDATE_FIELD(ctx->initiator_id, ctx->initiator_id_len);
    UPDATE_FIELD(ctx->responder_id, ctx->responder_id_len);
    update_u16(&state, ctx->protocol_version);
    update_u16(&state, ctx->suite_id);
    update_u16(&state, ctx->profile_id);
    UPDATE_FIELD(ctx->auth_instance_id, ctx->auth_instance_id_len);
    UPDATE_FIELD(ctx->auth_transcript_hash, ctx->auth_transcript_hash_len);
#undef UPDATE_FIELD

    (void)crypto_hash_sha256_final(&state, out);
    return AUTH_TLS_EXPORTER_CONTEXT_OK;
}
