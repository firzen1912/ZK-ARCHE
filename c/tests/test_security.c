/*
 * test_security.c - release-qualification security regressions.
 *
 * Mirrors the Rust/Python security tests with the production C API:
 * invalid point/scalar rejection, strict fixed payload decoding, transcript
 * binding for proofs, role proof binding, and session-key binding.
 */

#include "auth/iot_auth.h"
#include "auth/auth_crypto.h"
#include "auth/auth_payloads.h"
#include "auth/auth_proofs.h"
#include "auth/auth_transcript.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("  FAIL: %s\n", msg); failures++; } \
} while (0)

#define OK(expr) do { \
    auth_err_t _e = (expr); \
    if (_e != AUTH_OK) { \
        printf("  FAIL: %s -> 0x%04x %s\n", #expr, _e, auth_strerror(_e)); \
        failures++; return; \
    } \
} while (0)

static void wide_seed(uint8_t out[64], uint8_t seed)
{
    memset(out, seed, 64);
}

static void scalar_seed(uint8_t out[32], uint8_t seed)
{
    uint8_t wide[64];
    wide_seed(wide, seed);
    auth_scalar_from_wide(wide, out);
}

static void point_seed(uint8_t out[32], uint8_t seed)
{
    uint8_t s[32];
    scalar_seed(s, seed);
    OK(auth_scalarmult_base(out, s));
}

static auth_schnorr_proof_t proof_seed(uint8_t seed)
{
    auth_schnorr_proof_t p = {0};
    point_seed(p.a, seed);
    scalar_seed(p.s, (uint8_t)(seed + 1u));
    return p;
}

static void fill_rng_bytes(uint8_t *out, size_t n)
{
    for (size_t i = 0; i < n; ++i) out[i] = (uint8_t)(i * 73u + 19u);
}

static void test_invalid_encoding_and_strict_payloads(void)
{
    printf("== invalid encodings + strict payloads ==\n");

    uint8_t zeros[32] = {0};
    uint8_t ones[32];
    memset(ones, 0xff, sizeof ones);
    CHECK(auth_check_point(zeros) != AUTH_OK, "all-zero point rejected");
    CHECK(auth_check_point(ones) == AUTH_ERR_INVALID_POINT, "invalid point rejected");
    CHECK(auth_check_scalar(ones) == AUTH_ERR_NONCANONICAL_SCALAR, "non-canonical scalar rejected");

    uint8_t buf[2048];
    size_t w = 0;

    auth_setup1_t s1 = {0};
    memcpy(s1.pairing_token, "token", 5);
    s1.pairing_token_len = 5;
    memset(s1.device_id, 1, 32);
    point_seed(s1.device_pub, 2);
    memset(s1.client_nonce, 3, 32);
    point_seed(s1.role_commitment, 4);
    OK(auth_setup1_encode(&s1, buf, sizeof buf, &w));
    auth_setup1_t s1_out;
    CHECK(auth_setup1_decode(&s1_out, buf, w + 1) == AUTH_ERR_MALFORMED_PACKET,
          "SETUP_1 trailing byte rejected");

    auth_setup2_t s2 = {0};
    memset(s2.server_nonce, 5, 32);
    memset(s2.setup_challenge, 6, 16);
    point_seed(s2.server_pub, 7);
    s2.server_proof = proof_seed(8);
    OK(auth_setup2_encode(&s2, buf, sizeof buf, &w));
    auth_setup2_t s2_out;
    CHECK(auth_setup2_decode(&s2_out, buf, w + 1) == AUTH_ERR_MALFORMED_PACKET,
          "SETUP_2 trailing byte rejected");

    auth_setup3_t s3 = {0};
    s3.client_proof = proof_seed(10);
    OK(auth_setup3_encode(&s3, buf, sizeof buf, &w));
    auth_setup3_t s3_out;
    CHECK(auth_setup3_decode(&s3_out, buf, w + 1) == AUTH_ERR_MALFORMED_PACKET,
          "SETUP_3 trailing byte rejected");

    auth_auth1_t a1 = {0};
    memset(a1.pid, 11, 32);
    a1.client_proof = proof_seed(12);
    memset(a1.nonce_c, 13, 32);
    point_seed(a1.eph_c, 14);
    point_seed(a1.c_prime, 15);
    a1.rerand_proof = proof_seed(16);
    a1.n_branches = 1;
    point_seed(a1.branches[0].a, 17);
    scalar_seed(a1.branches[0].c, 18);
    scalar_seed(a1.branches[0].s, 19);
    OK(auth_auth1_encode(&a1, buf, sizeof buf, &w));
    auth_auth1_t a1_out;
    CHECK(auth_auth1_decode(&a1_out, buf, w + 1) == AUTH_ERR_MALFORMED_PACKET,
          "AUTH_1 trailing byte rejected");

    auth_auth2_t a2 = {0};
    point_seed(a2.server_pub, 20);
    a2.server_proof = proof_seed(21);
    memset(a2.nonce_s, 22, 32);
    point_seed(a2.eph_s, 23);
    memset(a2.tag_s, 24, 32);
    OK(auth_auth2_encode(&a2, buf, sizeof buf, &w));
    auth_auth2_t a2_out;
    CHECK(auth_auth2_decode(&a2_out, buf, w + 1) == AUTH_ERR_MALFORMED_PACKET,
          "AUTH_2 trailing byte rejected");

    auth_auth3_t a3 = {0};
    memset(a3.tag_c, 25, 32);
    OK(auth_auth3_encode(&a3, buf, sizeof buf, &w));
    auth_auth3_t a3_out;
    CHECK(auth_auth3_decode(&a3_out, buf, w + 1) == AUTH_ERR_MALFORMED_PACKET,
          "AUTH_3 trailing byte rejected");
}

static void test_auth_proof_binding(void)
{
    printf("== auth proof transcript binding ==\n");

    uint8_t x[32], device_pub[32], server_pub[32], eph_secret[32], eph_c[32];
    uint8_t nonce_c[32], pid[32], wide[64];
    scalar_seed(x, 31);
    OK(auth_scalarmult_base(device_pub, x));
    point_seed(server_pub, 32);
    scalar_seed(eph_secret, 33);
    OK(auth_scalarmult_base(eph_c, eph_secret));
    memset(nonce_c, 34, 32);
    OK(auth_compute_pid(pid, device_pub, nonce_c, eph_c, server_pub));

    auth_schnorr_proof_t proof;
    fill_rng_bytes(wide, sizeof wide);
    OK(auth_prove_auth_client_with_bytes(&proof, wide, x, pid, nonce_c, eph_c));
    CHECK(auth_verify_auth_client(&proof, device_pub, pid, nonce_c, eph_c),
          "valid auth proof verifies");

    uint8_t bad[32];
    memcpy(bad, pid, 32);
    bad[0] ^= 1;
    CHECK(!auth_verify_auth_client(&proof, device_pub, bad, nonce_c, eph_c),
          "pid mutation rejected");

    memcpy(bad, nonce_c, 32);
    bad[7] ^= 0x80;
    CHECK(!auth_verify_auth_client(&proof, device_pub, pid, bad, eph_c),
          "nonce mutation rejected");

    uint8_t other_pub[32], other_eph[32];
    point_seed(other_pub, 35);
    point_seed(other_eph, 36);
    CHECK(!auth_verify_auth_client(&proof, other_pub, pid, nonce_c, eph_c),
          "public-key mutation rejected");
    CHECK(!auth_verify_auth_client(&proof, device_pub, pid, nonce_c, other_eph),
          "ephemeral-key mutation rejected");

    auth_schnorr_proof_t bad_proof = proof;
    bad_proof.s[0] ^= 1;
    CHECK(!auth_verify_auth_client(&bad_proof, device_pub, pid, nonce_c, eph_c),
          "proof scalar mutation rejected");
}

static void test_role_and_session_binding(void)
{
    printf("== role proof + session key binding ==\n");

    uint8_t blind[32], delta[32], stored_c[32], c_prime[32], blind_prime[32];
    uint8_t pid[32], nonce_c[32], eph_c[32], wide[64 * 5];
    uint64_t allowed[3] = {1, 2, 3};
    scalar_seed(blind, 41);
    scalar_seed(delta, 42);
    OK(auth_make_role_commitment(stored_c, 2, blind));
    OK(auth_rerandomize_commitment(c_prime, blind_prime, stored_c, blind, delta));
    memset(pid, 43, 32);
    memset(nonce_c, 44, 32);
    point_seed(eph_c, 45);

    auth_schnorr_proof_t rerand;
    fill_rng_bytes(wide, 64);
    OK(auth_prove_rerandomization_with_bytes(&rerand, wide, stored_c, c_prime, delta,
                                             pid, nonce_c, eph_c));
    CHECK(auth_verify_rerandomization(&rerand, stored_c, c_prime, pid, nonce_c, eph_c),
          "valid rerandomization proof verifies");
    uint8_t bad_pid[32];
    memcpy(bad_pid, pid, 32);
    bad_pid[0] ^= 1;
    CHECK(!auth_verify_rerandomization(&rerand, stored_c, c_prime, bad_pid, nonce_c, eph_c),
          "rerandomization pid mutation rejected");

    auth_set_branch_t branches[AUTH_MAX_ROLES];
    size_t n_branches = 0;
    fill_rng_bytes(wide, sizeof wide);
    OK(auth_prove_role_set_membership_with_bytes(
        branches, &n_branches, wide, sizeof wide, allowed, 3, c_prime, 2,
        blind_prime, pid, nonce_c, eph_c));
    CHECK(n_branches == 3, "role proof branch count");
    CHECK(auth_verify_role_set_membership(branches, n_branches, allowed, 3,
                                          c_prime, pid, nonce_c, eph_c),
          "valid role proof verifies");

    uint64_t wrong_roles[3] = {1, 3, 4};
    CHECK(!auth_verify_role_set_membership(branches, n_branches, wrong_roles, 3,
                                           c_prime, pid, nonce_c, eph_c),
          "wrong role set rejected");
    auth_set_branch_t tampered[AUTH_MAX_ROLES];
    memcpy(tampered, branches, sizeof tampered);
    tampered[0].c[0] ^= 1;
    CHECK(!auth_verify_role_set_membership(tampered, n_branches, allowed, 3,
                                           c_prime, pid, nonce_c, eph_c),
          "tampered role branch rejected");

    uint8_t client_eph[32], server_eph[32], ec[32], es[32], nonce_s[32], key_c[32], key_s[32];
    scalar_seed(client_eph, 51);
    scalar_seed(server_eph, 52);
    OK(auth_scalarmult_base(ec, client_eph));
    OK(auth_scalarmult_base(es, server_eph));
    memset(nonce_s, 53, 32);
    OK(auth_derive_session_key(key_c, client_eph, es, nonce_c, nonce_s, pid, ec, es));
    OK(auth_derive_session_key(key_s, server_eph, ec, nonce_c, nonce_s, pid, ec, es));
    CHECK(memcmp(key_c, key_s, 32) == 0, "client/server session keys match");
    nonce_s[0] ^= 1;
    OK(auth_derive_session_key(key_s, client_eph, es, nonce_c, nonce_s, pid, ec, es));
    CHECK(memcmp(key_c, key_s, 32) != 0, "nonce_s mutation changes session key");
}

int main(void)
{
    if (auth_init() != AUTH_OK) { fprintf(stderr, "auth_init failed\n"); return 1; }
    test_invalid_encoding_and_strict_payloads();
    test_auth_proof_binding();
    test_role_and_session_binding();
    printf("\n%s: %d failure(s)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
