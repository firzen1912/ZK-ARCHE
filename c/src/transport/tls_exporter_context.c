#include "auth/tls_exporter_context.h"

#include <limits.h>
#include <sodium.h>

static const uint8_t domain[] = "ZKARCHE-EXPORTER-CONTEXT-v1";

static int required_field_invalid(const uint8_t *value, size_t len)
{
    return value == NULL || len == 0u;
}

static auth_tls_exporter_context_err_t update_len_field(
    crypto_hash_sha256_state *state, const uint8_t *value, size_t len)
{
    uint8_t len_be[2];
    if (len > UINT16_MAX) return AUTH_EXPORTER_CONTEXT_FIELD_TOO_LONG;
    len_be[0] = (uint8_t)(len >> 8);
    len_be[1] = (uint8_t)len;
    crypto_hash_sha256_update(state, len_be, sizeof len_be);
    if (len != 0u) crypto_hash_sha256_update(state, value, (unsigned long long)len);
    return AUTH_EXPORTER_CONTEXT_OK;
}

auth_tls_exporter_context_err_t auth_tls_exporter_context_digest(
    const auth_tls_exporter_context_t *ctx,
    int allow_empty_alpn,
    uint8_t out[AUTH_TLS_EXPORTER_CONTEXT_LEN])
{
    crypto_hash_sha256_state state;
    auth_tls_exporter_context_err_t err;
    uint8_t fixed[6];

    if (ctx == NULL || out == NULL ||
        required_field_invalid(ctx->application_id, ctx->application_id_len) ||
        required_field_invalid(ctx->deployment_id, ctx->deployment_id_len) ||
        required_field_invalid(ctx->initiator_id, ctx->initiator_id_len) ||
        required_field_invalid(ctx->responder_id, ctx->responder_id_len) ||
        required_field_invalid(ctx->auth_instance_id, ctx->auth_instance_id_len) ||
        required_field_invalid(ctx->auth_transcript_hash, ctx->auth_transcript_hash_len)) {
        return AUTH_EXPORTER_CONTEXT_EMPTY_REQUIRED_FIELD;
    }
    if (ctx->alpn_len != 0u && ctx->alpn == NULL) return AUTH_EXPORTER_CONTEXT_EMPTY_REQUIRED_FIELD;
    if (ctx->alpn_len == 0u && !allow_empty_alpn) return AUTH_EXPORTER_CONTEXT_EMPTY_ALPN_NOT_PERMITTED;

    if (crypto_hash_sha256_init(&state) != 0) return AUTH_EXPORTER_CONTEXT_EMPTY_REQUIRED_FIELD;
    crypto_hash_sha256_update(&state, domain, sizeof domain - 1u);

#define UPDATE_FIELD(ptr, len) do { err = update_len_field(&state, (ptr), (len)); if (err != AUTH_EXPORTER_CONTEXT_OK) return err; } while (0)
    UPDATE_FIELD(ctx->application_id, ctx->application_id_len);
    UPDATE_FIELD(ctx->alpn, ctx->alpn_len);
    UPDATE_FIELD(ctx->deployment_id, ctx->deployment_id_len);
    UPDATE_FIELD(ctx->initiator_id, ctx->initiator_id_len);
    UPDATE_FIELD(ctx->responder_id, ctx->responder_id_len);
#undef UPDATE_FIELD

    fixed[0] = (uint8_t)(ctx->protocol_version >> 8); fixed[1] = (uint8_t)ctx->protocol_version;
    fixed[2] = (uint8_t)(ctx->suite_id >> 8); fixed[3] = (uint8_t)ctx->suite_id;
    fixed[4] = (uint8_t)(ctx->profile_id >> 8); fixed[5] = (uint8_t)ctx->profile_id;
    crypto_hash_sha256_update(&state, fixed, sizeof fixed);

    err = update_len_field(&state, ctx->auth_instance_id, ctx->auth_instance_id_len);
    if (err != AUTH_EXPORTER_CONTEXT_OK) return err;
    err = update_len_field(&state, ctx->auth_transcript_hash, ctx->auth_transcript_hash_len);
    if (err != AUTH_EXPORTER_CONTEXT_OK) return err;

    crypto_hash_sha256_final(&state, out);
    return AUTH_EXPORTER_CONTEXT_OK;
}
