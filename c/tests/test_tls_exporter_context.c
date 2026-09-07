#include "auth/tls_exporter_context.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    static const uint8_t app[] = "zk-arche";
    static const uint8_t alpn[] = "zkarche/1";
    static const uint8_t deployment[] = "lab";
    static const uint8_t initiator[] = {0x01, 0x02};
    static const uint8_t responder[] = {0x03, 0x04};
    static const uint8_t instance[] = "auth-0001";
    static const uint8_t expected[32] = {
        0xa3,0xba,0x08,0x67,0xea,0xd3,0xc6,0x04,
        0x16,0xf1,0xa6,0x85,0x96,0x56,0x6f,0x01,
        0x73,0x9b,0x35,0x20,0xb4,0x14,0x22,0xa3,
        0xcd,0xa7,0xc0,0x39,0xb2,0x80,0xc1,0x07
    };
    uint8_t transcript[32];
    uint8_t out[32];
    auth_tls_exporter_context_v1_t ctx;
    size_t i;

    for (i = 0; i < sizeof transcript; ++i) transcript[i] = (uint8_t)i;
    ctx.application_id = app; ctx.application_id_len = sizeof app - 1u;
    ctx.alpn = alpn; ctx.alpn_len = sizeof alpn - 1u;
    ctx.deployment_id = deployment; ctx.deployment_id_len = sizeof deployment - 1u;
    ctx.initiator_id = initiator; ctx.initiator_id_len = sizeof initiator;
    ctx.responder_id = responder; ctx.responder_id_len = sizeof responder;
    ctx.protocol_version = 3u; ctx.suite_id = 1u; ctx.profile_id = 2u;
    ctx.auth_instance_id = instance; ctx.auth_instance_id_len = sizeof instance - 1u;
    ctx.auth_transcript_hash = transcript; ctx.auth_transcript_hash_len = sizeof transcript;

    assert(auth_tls_exporter_context_v1_digest(out, &ctx, false) == AUTH_TLS_EXPORTER_CONTEXT_OK);
    assert(memcmp(out, expected, sizeof expected) == 0);

    ctx.application_id_len = 0u;
    assert(auth_tls_exporter_context_v1_digest(out, &ctx, false) == AUTH_TLS_EXPORTER_CONTEXT_EMPTY_REQUIRED);
    ctx.application_id_len = sizeof app - 1u;
    ctx.alpn_len = 0u;
    assert(auth_tls_exporter_context_v1_digest(out, &ctx, false) == AUTH_TLS_EXPORTER_CONTEXT_EMPTY_ALPN);
    assert(auth_tls_exporter_context_v1_digest(out, &ctx, true) == AUTH_TLS_EXPORTER_CONTEXT_OK);

    puts("tls exporter context: ok");
    return 0;
}
