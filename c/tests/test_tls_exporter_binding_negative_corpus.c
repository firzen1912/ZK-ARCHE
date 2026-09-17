#include "auth/tls_exporter_context.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CORPUS_PATH "../rust/test-vectors/transport/tls-exporter-binding-negatives-v1.txt"

static void init_base(auth_tls_exporter_context_v1_t *ctx,
                      uint8_t transcript[32],
                      const uint8_t **app, size_t *app_len,
                      const uint8_t **initiator, size_t *initiator_len,
                      const uint8_t **responder, size_t *responder_len,
                      const uint8_t **instance, size_t *instance_len) {
    static const uint8_t base_app[] = "zk-arche";
    static const uint8_t alpn[] = "zkarche/1";
    static const uint8_t deployment[] = "lab";
    static const uint8_t base_initiator[] = {0x01, 0x02};
    static const uint8_t base_responder[] = {0x03, 0x04};
    static const uint8_t base_instance[] = "auth-0001";
    size_t i;

    for (i = 0; i < 32u; ++i) transcript[i] = (uint8_t)i;
    *app = base_app; *app_len = sizeof base_app - 1u;
    *initiator = base_initiator; *initiator_len = sizeof base_initiator;
    *responder = base_responder; *responder_len = sizeof base_responder;
    *instance = base_instance; *instance_len = sizeof base_instance - 1u;

    ctx->application_id = *app; ctx->application_id_len = *app_len;
    ctx->alpn = alpn; ctx->alpn_len = sizeof alpn - 1u;
    ctx->deployment_id = deployment; ctx->deployment_id_len = sizeof deployment - 1u;
    ctx->initiator_id = *initiator; ctx->initiator_id_len = *initiator_len;
    ctx->responder_id = *responder; ctx->responder_id_len = *responder_len;
    ctx->protocol_version = 3u; ctx->suite_id = 1u; ctx->profile_id = 2u;
    ctx->auth_instance_id = *instance; ctx->auth_instance_id_len = *instance_len;
    ctx->auth_transcript_hash = transcript; ctx->auth_transcript_hash_len = 32u;
}

int main(void) {
    static const char *expected_ids[] = {
        "TB-N01", "TB-N02", "TB-N03", "TB-N04", "TB-N05",
        "TB-N06", "TB-N07", "TB-N08", "TB-N09", "TB-N10"
    };
    static const uint8_t alt_app[] = "zk-arche-alt";
    static const uint8_t alt_initiator[] = {0x05, 0x06};
    static const uint8_t alt_responder[] = {0x07, 0x08};
    static const uint8_t alt_instance[] = "auth-0002";
    static const uint8_t resumed_instance[] = "auth-resumed-0001";
    FILE *fp = fopen(CORPUS_PATH, "r");
    char line[512];
    uint8_t transcript[32], base[32], out[32];
    auth_tls_exporter_context_v1_t ctx;
    const uint8_t *app, *initiator, *responder, *instance;
    size_t app_len, initiator_len, responder_len, instance_len;
    size_t case_index = 0u;

    assert(fp != NULL);
    init_base(&ctx, transcript, &app, &app_len, &initiator, &initiator_len,
              &responder, &responder_len, &instance, &instance_len);
    assert(auth_tls_exporter_context_v1_digest(base, &ctx, false) == AUTH_TLS_EXPORTER_CONTEXT_OK);

    while (fgets(line, sizeof line, fp) != NULL) {
        char *id, *mutation, *relation, *intent, *save;
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        line[strcspn(line, "\r\n")] = '\0';
        save = NULL;
        id = strtok_r(line, "|", &save);
        mutation = strtok_r(NULL, "|", &save);
        relation = strtok_r(NULL, "|", &save);
        intent = strtok_r(NULL, "|", &save);
        assert(id != NULL && mutation != NULL && relation != NULL && intent != NULL);
        assert(case_index < (sizeof expected_ids / sizeof expected_ids[0]));
        assert(strcmp(id, expected_ids[case_index]) == 0);

        init_base(&ctx, transcript, &app, &app_len, &initiator, &initiator_len,
                  &responder, &responder_len, &instance, &instance_len);
        if (strcmp(mutation, "none") == 0 ||
            strcmp(mutation, "transport_address=changed") == 0 ||
            strcmp(mutation, "routing_metadata=changed") == 0) {
            /* Transport/routing metadata is deliberately outside authenticated context. */
        } else if (strcmp(mutation, "auth_instance_id=auth-0002") == 0) {
            ctx.auth_instance_id = alt_instance;
            ctx.auth_instance_id_len = sizeof alt_instance - 1u;
        } else if (strcmp(mutation, "auth_instance_id=auth-resumed-0001") == 0) {
            ctx.auth_instance_id = resumed_instance;
            ctx.auth_instance_id_len = sizeof resumed_instance - 1u;
        } else if (strcmp(mutation, "application_id=zk-arche-alt") == 0) {
            ctx.application_id = alt_app;
            ctx.application_id_len = sizeof alt_app - 1u;
        } else if (strcmp(mutation, "initiator_id=0506") == 0) {
            ctx.initiator_id = alt_initiator;
            ctx.initiator_id_len = sizeof alt_initiator;
        } else if (strcmp(mutation, "responder_id=0708") == 0) {
            ctx.responder_id = alt_responder;
            ctx.responder_id_len = sizeof alt_responder;
        } else if (strcmp(mutation, "profile_id=0003") == 0) {
            ctx.profile_id = 3u;
        } else {
            assert(!"unknown governed TLS exporter mutation");
        }

        assert(auth_tls_exporter_context_v1_digest(out, &ctx, false) == AUTH_TLS_EXPORTER_CONTEXT_OK);
        if (strcmp(relation, "equal_base") == 0) {
            assert(memcmp(out, base, sizeof base) == 0);
        } else if (strcmp(relation, "different_base") == 0) {
            assert(memcmp(out, base, sizeof base) != 0);
        } else {
            assert(!"unknown governed TLS exporter relation");
        }
        ++case_index;
    }
    assert(ferror(fp) == 0);
    assert(fclose(fp) == 0);
    assert(case_index == (sizeof expected_ids / sizeof expected_ids[0]));
    puts("tls exporter negative corpus: ok");
    return 0;
}
