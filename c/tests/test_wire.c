/*
 * test_wire.c — unit tests for the wire format and payload codecs.
 *
 * Covers:
 *   - 24-byte header encode/decode roundtrip
 *   - TLV read/write roundtrip
 *   - Hello encode/decode roundtrip with unknown TLV ignored
 *   - Capability negotiation edge cases
 *   - Each payload type (SETUP_1/2/3, AUTH_1/2/3, ACK, ERROR) roundtrips
 */

#include "auth/iot_auth.h"
#include "auth/auth_wire.h"
#include "auth/auth_payloads.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

static int failures = 0;
#define CHECK(expr, msg) do { \
    if (!(expr)) { printf("  FAIL: %s\n", msg); failures++; } \
} while (0)

#define OK(expr) CHECK((expr) == AUTH_OK, #expr " returned error")

/* ---- Header roundtrip ---- */

static void test_header_roundtrip(void)
{
    printf("== header roundtrip ==\n");

    auth_header_t in = {
        .version  = 0x02,
        .pkt_type = AUTH_PKT_AUTH_1,
        .flags    = AUTH_FLAG_RETRANSMIT,
        .seq      = 0xDEADBEEF,
    };
    for (int i = 0; i < 16; ++i) in.session_id[i] = (uint8_t)(0x10 + i);

    uint8_t buf[128];
    size_t written = 0;
    OK(auth_header_encode(&in, buf, sizeof buf, &written));
    CHECK(written == AUTH_HEADER_LEN, "written == 24");

    /* Append a dummy 10-byte payload to ensure the header decoder
     * returns the right pointer/length for the payload. */
    for (int i = 0; i < 10; ++i) buf[24 + i] = (uint8_t)i;

    auth_header_t out;
    const uint8_t *payload = NULL;
    size_t payload_len = 0;
    OK(auth_header_decode(&out, buf, 24 + 10, &payload, &payload_len));

    CHECK(out.version  == in.version,  "version");
    CHECK(out.pkt_type == in.pkt_type, "pkt_type");
    CHECK(out.flags    == in.flags,    "flags");
    CHECK(out.seq      == in.seq,      "seq");
    CHECK(memcmp(out.session_id, in.session_id, 16) == 0, "session_id");
    CHECK(payload_len == 10,            "payload_len");
    CHECK(payload == buf + 24,          "payload ptr");
    for (int i = 0; i < 10; ++i) CHECK(payload[i] == (uint8_t)i, "payload byte");
}

/* ---- TLV roundtrip ---- */

static void test_tlv_roundtrip(void)
{
    printf("== TLV roundtrip ==\n");

    uint8_t buf[256];
    size_t off = 0;

    uint8_t value1[] = { 0x11, 0x22, 0x33 };
    uint8_t value2[] = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE };
    OK(auth_tlv_write(buf, sizeof buf, &off, 0x1234, value1, sizeof value1));
    OK(auth_tlv_write(buf, sizeof buf, &off, 0x5678, value2, sizeof value2));

    size_t used = off;

    /* Read back */
    off = 0;
    uint16_t tag;
    const uint8_t *v;
    size_t vl;
    OK(auth_tlv_read(buf, used, &off, &tag, &v, &vl));
    CHECK(tag == 0x1234 && vl == 3 && memcmp(v, value1, 3) == 0, "tlv1");
    OK(auth_tlv_read(buf, used, &off, &tag, &v, &vl));
    CHECK(tag == 0x5678 && vl == 5 && memcmp(v, value2, 5) == 0, "tlv2");
    CHECK(auth_tlv_read(buf, used, &off, &tag, &v, &vl) ==
          AUTH_ERR_PAYLOAD_TOO_SHORT, "end-of-stream");
}

/* ---- Hello roundtrip with unknown TLV ---- */

static void test_hello_roundtrip(void)
{
    printf("== Hello roundtrip ==\n");

    auth_hello_t in = {
        .version     = AUTH_PROTOCOL_VERSION,
        .min_version = AUTH_MIN_SUPPORTED_VERSION,
        .suites      = { AUTH_SUITE_RISTRETTO255_SHA256 },
        .n_suites    = 1,
        .caps        = AUTH_CAPS_BASELINE,
        .has_mtu_hint = 1,
        .mtu_hint    = 1400,
    };
    uint8_t vid[] = { 'a','c','m','e' };
    memcpy(in.vendor_id, vid, sizeof vid);
    in.vendor_id_len = sizeof vid;
    const char *dm = "widget-v7";
    memcpy(in.device_model, dm, strlen(dm));
    in.device_model_len = strlen(dm);

    uint8_t buf[256];
    size_t written = 0;
    OK(auth_hello_encode(&in, buf, sizeof buf, &written));

    /* Splice in an UNKNOWN TLV at the end so we can prove the decoder
     * silently skips it. */
    uint8_t garbage[] = { 0xFA, 0xCE };
    size_t off = written;
    OK(auth_tlv_write(buf, sizeof buf, &off, 0xFF00, garbage, sizeof garbage));
    size_t total = off;

    auth_hello_t out;
    OK(auth_hello_decode(&out, buf, total));
    CHECK(out.version     == in.version,     "version");
    CHECK(out.min_version == in.min_version, "min_version");
    CHECK(out.n_suites    == 1 && out.suites[0] == AUTH_SUITE_RISTRETTO255_SHA256, "suite");
    CHECK(out.caps        == in.caps,        "caps");
    CHECK(out.has_mtu_hint && out.mtu_hint == 1400, "mtu_hint");
    CHECK(out.vendor_id_len == 4 && memcmp(out.vendor_id, "acme", 4) == 0, "vendor_id");
    CHECK(out.device_model_len == strlen(dm) &&
          memcmp(out.device_model, dm, strlen(dm)) == 0, "device_model");
}

/* ---- Negotiation ---- */

static void test_negotiate(void)
{
    printf("== negotiate ==\n");

    auth_suite_t suites[] = { AUTH_SUITE_RISTRETTO255_SHA256 };

    /* Happy path */
    auth_negotiated_t out;
    OK(auth_negotiate(&out,
        AUTH_PROTOCOL_VERSION, AUTH_MIN_SUPPORTED_VERSION,
        suites, 1, AUTH_CAPS_BASELINE,
        AUTH_PROTOCOL_VERSION, AUTH_MIN_SUPPORTED_VERSION,
        suites, 1, AUTH_CAPS_BASELINE));
    CHECK(out.version == AUTH_PROTOCOL_VERSION, "nego version");
    CHECK(out.suite   == AUTH_SUITE_RISTRETTO255_SHA256, "nego suite");
    CHECK((out.caps & AUTH_CAP_AUTH_V2) != 0, "nego caps");

    /* Peer too old: peer claims max 0x01 */
    auth_err_t e = auth_negotiate(&out,
        AUTH_PROTOCOL_VERSION, AUTH_MIN_SUPPORTED_VERSION,
        suites, 1, AUTH_CAPS_BASELINE,
        0x01, 0x01,
        suites, 1, AUTH_CAPS_BASELINE);
    CHECK(e == AUTH_ERR_UNSUPPORTED_VERSION, "old peer rejected");

    /* No shared suite */
    auth_suite_t exotic[] = { (auth_suite_t)0xBEEF };
    e = auth_negotiate(&out,
        AUTH_PROTOCOL_VERSION, AUTH_MIN_SUPPORTED_VERSION,
        suites, 1, AUTH_CAPS_BASELINE,
        AUTH_PROTOCOL_VERSION, AUTH_MIN_SUPPORTED_VERSION,
        exotic, 1, AUTH_CAPS_BASELINE);
    CHECK(e == AUTH_ERR_UNSUPPORTED_SUITE, "no shared suite");

    /* Missing AUTH_V2 in peer caps */
    e = auth_negotiate(&out,
        AUTH_PROTOCOL_VERSION, AUTH_MIN_SUPPORTED_VERSION,
        suites, 1, AUTH_CAPS_BASELINE,
        AUTH_PROTOCOL_VERSION, AUTH_MIN_SUPPORTED_VERSION,
        suites, 1, 0 /* no caps */);
    CHECK(e == AUTH_ERR_CAP_MISMATCH, "no AUTH_V2");
}

/* ---- Payload roundtrips ---- */

static void test_setup_payloads(void)
{
    printf("== SETUP_1/2/3 payload roundtrip ==\n");
    auth_setup1_t s1 = {0};
    memcpy(s1.pairing_token, "hello", 5); s1.pairing_token_len = 5;
    for (size_t i = 0; i < 32; ++i) {
        s1.device_id[i]       = (uint8_t)(0x10 + i);
        s1.device_pub[i]      = (uint8_t)(0x20 + i);
        s1.client_nonce[i]    = (uint8_t)(0x30 + i);
        s1.role_commitment[i] = (uint8_t)(0x40 + i);
    }
    uint8_t buf[512];
    size_t w = 0;
    OK(auth_setup1_encode(&s1, buf, sizeof buf, &w));
    auth_setup1_t s1b;
    OK(auth_setup1_decode(&s1b, buf, w));
    CHECK(s1b.pairing_token_len == 5 && memcmp(s1b.pairing_token, "hello", 5) == 0, "s1 token");
    CHECK(memcmp(s1b.device_id,       s1.device_id,       32) == 0, "s1 device_id");
    CHECK(memcmp(s1b.device_pub,      s1.device_pub,      32) == 0, "s1 device_pub");
    CHECK(memcmp(s1b.client_nonce,    s1.client_nonce,    32) == 0, "s1 client_nonce");
    CHECK(memcmp(s1b.role_commitment, s1.role_commitment, 32) == 0, "s1 role_commit");

    auth_setup2_t s2 = {0};
    for (size_t i = 0; i < 32; ++i) {
        s2.server_nonce[i]    = (uint8_t)(0x50 + i);
        s2.server_pub[i]      = (uint8_t)(0x60 + i);
        s2.server_proof.a[i]  = (uint8_t)(0x70 + i);
        s2.server_proof.s[i]  = (uint8_t)(0x80 + i);
    }
    for (size_t i = 0; i < 16; ++i) s2.setup_challenge[i] = (uint8_t)(0x90 + i);
    OK(auth_setup2_encode(&s2, buf, sizeof buf, &w));
    auth_setup2_t s2b;
    OK(auth_setup2_decode(&s2b, buf, w));
    CHECK(memcmp(s2b.server_nonce,    s2.server_nonce,    32) == 0, "s2 server_nonce");
    CHECK(memcmp(s2b.setup_challenge, s2.setup_challenge, 16) == 0, "s2 setup_challenge");
    CHECK(memcmp(s2b.server_pub,      s2.server_pub,      32) == 0, "s2 server_pub");
    CHECK(memcmp(s2b.server_proof.a,  s2.server_proof.a,  32) == 0, "s2 proof a");
    CHECK(memcmp(s2b.server_proof.s,  s2.server_proof.s,  32) == 0, "s2 proof s");

    auth_setup3_t s3 = {0};
    for (size_t i = 0; i < 32; ++i) {
        s3.client_proof.a[i] = (uint8_t)(0xA0 + i);
        s3.client_proof.s[i] = (uint8_t)(0xB0 + i);
    }
    OK(auth_setup3_encode(&s3, buf, sizeof buf, &w));
    auth_setup3_t s3b;
    OK(auth_setup3_decode(&s3b, buf, w));
    CHECK(memcmp(s3b.client_proof.a, s3.client_proof.a, 32) == 0, "s3 proof a");
    CHECK(memcmp(s3b.client_proof.s, s3.client_proof.s, 32) == 0, "s3 proof s");
}

static void test_auth_payloads(void)
{
    printf("== AUTH_1/2/3 payload roundtrip ==\n");

    auth_auth1_t a1 = {0};
    for (size_t i = 0; i < 32; ++i) {
        a1.pid[i]              = (uint8_t)(0x01 + i);
        a1.client_proof.a[i]   = (uint8_t)(0x02 + i);
        a1.client_proof.s[i]   = (uint8_t)(0x03 + i);
        a1.nonce_c[i]          = (uint8_t)(0x04 + i);
        a1.eph_c[i]            = (uint8_t)(0x05 + i);
        a1.c_prime[i]          = (uint8_t)(0x06 + i);
        a1.rerand_proof.a[i]   = (uint8_t)(0x07 + i);
        a1.rerand_proof.s[i]   = (uint8_t)(0x08 + i);
    }
    a1.n_branches = 3;
    for (size_t bi = 0; bi < 3; ++bi) {
        for (size_t i = 0; i < 32; ++i) {
            a1.branches[bi].a[i] = (uint8_t)(0x40 + bi * 3 + i);
            a1.branches[bi].c[i] = (uint8_t)(0x50 + bi * 3 + i);
            a1.branches[bi].s[i] = (uint8_t)(0x60 + bi * 3 + i);
        }
    }
    uint8_t buf[2048];
    size_t w = 0;
    OK(auth_auth1_encode(&a1, buf, sizeof buf, &w));
    auth_auth1_t a1b;
    OK(auth_auth1_decode(&a1b, buf, w));
    CHECK(memcmp(a1b.pid,            a1.pid,            32) == 0, "a1 pid");
    CHECK(memcmp(a1b.client_proof.a, a1.client_proof.a, 32) == 0, "a1 cp a");
    CHECK(memcmp(a1b.client_proof.s, a1.client_proof.s, 32) == 0, "a1 cp s");
    CHECK(memcmp(a1b.nonce_c,        a1.nonce_c,        32) == 0, "a1 nonce_c");
    CHECK(memcmp(a1b.eph_c,          a1.eph_c,          32) == 0, "a1 eph_c");
    CHECK(memcmp(a1b.c_prime,        a1.c_prime,        32) == 0, "a1 c_prime");
    CHECK(memcmp(a1b.rerand_proof.a, a1.rerand_proof.a, 32) == 0, "a1 rp a");
    CHECK(memcmp(a1b.rerand_proof.s, a1.rerand_proof.s, 32) == 0, "a1 rp s");
    CHECK(a1b.n_branches == 3,                                    "a1 n_branches");
    for (size_t bi = 0; bi < 3; ++bi) {
        CHECK(memcmp(a1b.branches[bi].a, a1.branches[bi].a, 32) == 0, "a1 br.a");
        CHECK(memcmp(a1b.branches[bi].c, a1.branches[bi].c, 32) == 0, "a1 br.c");
        CHECK(memcmp(a1b.branches[bi].s, a1.branches[bi].s, 32) == 0, "a1 br.s");
    }

    auth_auth2_t a2 = {0};
    for (size_t i = 0; i < 32; ++i) {
        a2.server_pub[i]     = (uint8_t)(0x11 + i);
        a2.server_proof.a[i] = (uint8_t)(0x22 + i);
        a2.server_proof.s[i] = (uint8_t)(0x33 + i);
        a2.nonce_s[i]        = (uint8_t)(0x44 + i);
        a2.eph_s[i]          = (uint8_t)(0x55 + i);
        a2.tag_s[i]          = (uint8_t)(0x66 + i);
    }
    OK(auth_auth2_encode(&a2, buf, sizeof buf, &w));
    auth_auth2_t a2b;
    OK(auth_auth2_decode(&a2b, buf, w));
    CHECK(memcmp(a2b.server_pub,     a2.server_pub,     32) == 0, "a2 server_pub");
    CHECK(memcmp(a2b.server_proof.a, a2.server_proof.a, 32) == 0, "a2 sp a");
    CHECK(memcmp(a2b.server_proof.s, a2.server_proof.s, 32) == 0, "a2 sp s");
    CHECK(memcmp(a2b.nonce_s,        a2.nonce_s,        32) == 0, "a2 nonce_s");
    CHECK(memcmp(a2b.eph_s,          a2.eph_s,          32) == 0, "a2 eph_s");
    CHECK(memcmp(a2b.tag_s,          a2.tag_s,          32) == 0, "a2 tag_s");

    auth_auth3_t a3 = {0};
    for (size_t i = 0; i < 32; ++i) a3.tag_c[i] = (uint8_t)(0x77 + i);
    OK(auth_auth3_encode(&a3, buf, sizeof buf, &w));
    auth_auth3_t a3b;
    OK(auth_auth3_decode(&a3b, buf, w));
    CHECK(memcmp(a3b.tag_c, a3.tag_c, 32) == 0, "a3 tag_c");
}

/* ---- ACK + ERROR ---- */

static void test_ack_and_error(void)
{
    printf("== ACK + ERROR ==\n");

    /* ACK */
    uint8_t ack[2];
    size_t w = 0;
    OK(auth_ack_encode(ack, sizeof ack, &w));
    CHECK(w == 1 && ack[0] == AUTH_ACK_BYTE, "ack byte");
    OK(auth_ack_decode(ack, 1));

    /* ERROR packet */
    uint8_t buf[256];
    uint8_t sid[16] = {0};
    size_t pkt_len = 0;
    OK(auth_packet_build_error(sid, 7, AUTH_ERR_REPLAY_DETECTED,
                                   "nope", buf, sizeof buf, &pkt_len));

    auth_header_t h;
    const uint8_t *p;
    size_t pl;
    OK(auth_header_decode(&h, buf, pkt_len, &p, &pl));
    CHECK(h.pkt_type == AUTH_PKT_ERROR, "is ERROR");
    auth_err_t code;
    const char *msg;
    size_t msg_len;
    OK(auth_packet_parse_error(p, pl, &code, &msg, &msg_len));
    CHECK(code == AUTH_ERR_REPLAY_DETECTED, "code");
    CHECK(msg_len == 4 && memcmp(msg, "nope", 4) == 0, "msg");
}

/* ---- Main ---- */

int main(void)
{
    if (auth_init() != AUTH_OK) { fprintf(stderr, "init fail\n"); return 1; }

    test_header_roundtrip();
    test_tlv_roundtrip();
    test_hello_roundtrip();
    test_negotiate();
    test_setup_payloads();
    test_auth_payloads();
    test_ack_and_error();

    printf("\n%s: %d failure(s)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
