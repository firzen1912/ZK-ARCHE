#include "auth/tls_exporter_context.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static auth_tls_exporter_context_v1_t fixture(const uint8_t transcript[32])
{
    static const uint8_t application_id[] = "zk-arche";
    static const uint8_t alpn[] = "zkarche/1";
    static const uint8_t deployment_id[] = "lab";
    static const uint8_t initiator_id[] = {0x01, 0x02};
    static const uint8_t responder_id[] = {0x03, 0x04};
    static const uint8_t auth_instance_id[] = "auth-0001";
    auth_tls_exporter_context_v1_t ctx = {
        application_id, sizeof application_id - 1u,
        alpn, sizeof alpn - 1u,
        deployment_id, sizeof deployment_id - 1u,
        initiator_id, sizeof initiator_id,
        responder_id, sizeof responder_id,
        3u, 1u, 2u,
        auth_instance_id, sizeof auth_instance_id - 1u,
        transcript, 32u
    };
    return ctx;
}

static void assert_context_changes(
    const uint8_t expected[32],
    const auth_tls_exporter_context_v1_t *ctx)
{
    uint8_t changed[32];
    assert(auth_tls_exporter_context_v1_digest(changed, ctx, false) == AUTH_TLS_EXPORTER_CONTEXT_OK);
    assert(memcmp(expected, changed, 32u) != 0);
}

int main(void)
{
    static const uint8_t expected[32] = {
        0xa3,0xba,0x08,0x67,0xea,0xd3,0xc6,0x04,0x16,0xf1,0xa6,0x85,0x96,0x56,0x6f,0x01,
        0x73,0x9b,0x35,0x20,0xb4,0x14,0x22,0xa3,0xcd,0xa7,0xc0,0x39,0xb2,0x80,0xc1,0x07
    };
    static const uint8_t other_deployment[] = "field";
    uint8_t transcript[32];
    uint8_t alternate_transcript[32];
    uint8_t digest[32];
    uint8_t changed[32];
    auth_tls_exporter_context_v1_t ctx;
    size_t i;

    for (i = 0u; i < sizeof transcript; ++i) {
        transcript[i] = (uint8_t)i;
        alternate_transcript[i] = (uint8_t)i;
    }
    alternate_transcript[31] ^= 0x01u;

    ctx = fixture(transcript);
    assert(auth_tls_exporter_context_v1_digest(digest, &ctx, false) == AUTH_TLS_EXPORTER_CONTEXT_OK);
    assert(memcmp(digest, expected, sizeof expected) == 0);

    {
        const uint8_t *saved = ctx.initiator_id;
        size_t saved_len = ctx.initiator_id_len;
        ctx.initiator_id = ctx.responder_id; ctx.initiator_id_len = ctx.responder_id_len;
        ctx.responder_id = saved; ctx.responder_id_len = saved_len;
        assert_context_changes(digest, &ctx);
    }

    ctx = fixture(transcript);
    ctx.auth_instance_id = (const uint8_t *)"auth-0002";
    assert_context_changes(digest, &ctx);

    ctx = fixture(transcript);
    ctx.deployment_id = other_deployment;
    ctx.deployment_id_len = sizeof other_deployment - 1u;
    assert_context_changes(digest, &ctx);

    ctx = fixture(transcript);
    ctx.suite_id = 2u;
    assert_context_changes(digest, &ctx);

    ctx = fixture(transcript);
    ctx.profile_id = 3u;
    assert_context_changes(digest, &ctx);

    ctx = fixture(alternate_transcript);
    assert_context_changes(digest, &ctx);

    ctx = fixture(transcript);
    ctx.application_id_len = 0u;
    assert(auth_tls_exporter_context_v1_digest(changed, &ctx, false) == AUTH_TLS_EXPORTER_CONTEXT_EMPTY_REQUIRED);

    ctx = fixture(transcript);
    ctx.alpn = NULL; ctx.alpn_len = 0u;
    assert(auth_tls_exporter_context_v1_digest(changed, &ctx, false) == AUTH_TLS_EXPORTER_CONTEXT_EMPTY_ALPN);
    assert(auth_tls_exporter_context_v1_digest(changed, &ctx, true) == AUTH_TLS_EXPORTER_CONTEXT_OK);

    puts("tls exporter context: ok");
    return 0;
}
