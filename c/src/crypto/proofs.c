/*
 * auth_proofs.c — Schnorr, rerandomization, and CDS-OR set-membership
 * proofs. Matches the Rust auth_proto::crypto module byte-for-byte.
 *
 * Every prove function has a _with_bytes variant taking pre-drawn
 * randomness (64 "wide" bytes per scalar), for deterministic test
 * vectors and hardware-RNG plumbing.
 */

#include "auth/auth_proofs.h"
#include "auth/auth_transcript.h"
#include "auth/auth_crypto.h"

#include <sodium.h>
#include <stdio.h>
#include <string.h>

/* ---- Common helpers ---- */

/* Verify A == B + C*s — used to check Schnorr equations. Returns 1 if
 * equal, 0 otherwise. Constant-time comparison via sodium_memcmp. */
static int points_equal(const uint8_t a[32], const uint8_t b[32])
{
    return sodium_memcmp(a, b, 32) == 0;
}

/* Compute g*s on ristretto255 basepoint. */
static auth_err_t g_times(uint8_t out[32], const uint8_t s[32])
{
    return auth_scalarmult_base(out, s);
}

/* Compute point*s. */
static auth_err_t p_times(uint8_t out[32], const uint8_t s[32],
                              const uint8_t p[32])
{
    return auth_scalarmult(out, s, p);
}

/* ---- Setup-client Schnorr proof ---- */

/* Build the setup-client transcript and finalize as a challenge scalar. */
static auth_err_t setup_client_challenge(
    uint8_t c_out[32],
    const uint8_t device_id[32],
    const uint8_t device_pub[32],
    const uint8_t server_pub[32],
    const uint8_t a[32],
    const uint8_t client_nonce[32],
    const uint8_t server_nonce[32],
    const uint8_t setup_challenge_in[AUTH_SETUP_CHALLENGE_LEN])
{
    size_t dom_len;
    const uint8_t *dom = auth_ds_setup(&dom_len);

    auth_transcript_t t;
    auth_err_t err = auth_transcript_init(&t, dom, dom_len);
    if (err) return err;
    err = auth_transcript_append       (&t, (const uint8_t *)"role",       4, (const uint8_t *)"client", 6); if (err) return err;
    err = auth_transcript_append       (&t, (const uint8_t *)"device_id",  9, device_id, 32);               if (err) return err;
    err = auth_transcript_append_point (&t, (const uint8_t *)"device_pub", 10, device_pub);                 if (err) return err;
    err = auth_transcript_append_point (&t, (const uint8_t *)"server_pub", 10, server_pub);                 if (err) return err;
    err = auth_transcript_append_point (&t, (const uint8_t *)"a",          1,  a);                          if (err) return err;
    err = auth_transcript_append       (&t, (const uint8_t *)"client_nonce", 12, client_nonce, 32);         if (err) return err;
    err = auth_transcript_append       (&t, (const uint8_t *)"server_nonce", 12, server_nonce, 32);         if (err) return err;
    err = auth_transcript_append       (&t, (const uint8_t *)"setup_challenge", 15,
                                             setup_challenge_in, AUTH_SETUP_CHALLENGE_LEN);                 if (err) return err;
    return auth_transcript_challenge(&t, c_out);
}

auth_err_t auth_prove_setup_client_with_bytes(
    auth_schnorr_proof_t *proof,
    const uint8_t wide_r[64],
    const uint8_t x[32],
    const uint8_t device_id[32],
    const uint8_t device_pub[32],
    const uint8_t server_pub[32],
    const uint8_t client_nonce[32],
    const uint8_t server_nonce[32],
    const uint8_t setup_challenge_in[AUTH_SETUP_CHALLENGE_LEN])
{
    uint8_t r[32];
    auth_scalar_from_wide(wide_r, r);

    auth_err_t err = g_times(proof->a, r);
    if (err) { sodium_memzero(r, sizeof r); return err; }

    uint8_t c[32];
    err = setup_client_challenge(c, device_id, device_pub, server_pub,
                                 proof->a, client_nonce, server_nonce,
                                 setup_challenge_in);
    if (err) { sodium_memzero(r, sizeof r); return err; }

    /* s = r + c*x */
    uint8_t cx[32];
    auth_scalar_mul(cx, c, x);
    auth_scalar_add(proof->s, r, cx);

    sodium_memzero(r, sizeof r);
    sodium_memzero(cx, sizeof cx);
    sodium_memzero(c,  sizeof c);
    return AUTH_OK;
}

auth_err_t auth_prove_setup_client(
    auth_schnorr_proof_t *proof,
    const uint8_t x[32],
    const uint8_t device_id[32],
    const uint8_t device_pub[32],
    const uint8_t server_pub[32],
    const uint8_t client_nonce[32],
    const uint8_t server_nonce[32],
    const uint8_t setup_challenge_in[AUTH_SETUP_CHALLENGE_LEN])
{
    uint8_t wide[64];
    randombytes_buf(wide, sizeof wide);
    auth_err_t err = auth_prove_setup_client_with_bytes(
        proof, wide, x, device_id, device_pub, server_pub,
        client_nonce, server_nonce, setup_challenge_in);
    sodium_memzero(wide, sizeof wide);
    return err;
}

int auth_verify_setup_client(
    const auth_schnorr_proof_t *proof,
    const uint8_t device_id[32],
    const uint8_t device_pub[32],
    const uint8_t server_pub[32],
    const uint8_t client_nonce[32],
    const uint8_t server_nonce[32],
    const uint8_t setup_challenge_in[AUTH_SETUP_CHALLENGE_LEN])
{
    uint8_t c[32];
    if (setup_client_challenge(c, device_id, device_pub, server_pub,
                               proof->a, client_nonce, server_nonce,
                               setup_challenge_in) != AUTH_OK) return 0;
    /* Check g*s == a + device_pub*c */
    uint8_t gs[32], pc[32], rhs[32];
    if (g_times(gs, proof->s) != AUTH_OK) return 0;
    if (p_times(pc, c, device_pub) != AUTH_OK) return 0;
    if (auth_point_add(rhs, proof->a, pc) != AUTH_OK) return 0;
    return points_equal(gs, rhs);
}

/* ---- Setup-server Schnorr proof ---- */

static auth_err_t setup_server_challenge(
    uint8_t c_out[32],
    const uint8_t device_id[32],
    const uint8_t device_pub[32],
    const uint8_t server_pub[32],
    const uint8_t a[32],
    const uint8_t client_nonce[32],
    const uint8_t server_nonce[32],
    const uint8_t setup_challenge_in[AUTH_SETUP_CHALLENGE_LEN])
{
    size_t dom_len;
    const uint8_t *dom = auth_ds_setup_server(&dom_len);

    auth_transcript_t t;
    auth_err_t err = auth_transcript_init(&t, dom, dom_len);
    if (err) return err;
    err = auth_transcript_append       (&t, (const uint8_t *)"role",       4, (const uint8_t *)"server", 6); if (err) return err;
    err = auth_transcript_append       (&t, (const uint8_t *)"device_id",  9, device_id, 32);               if (err) return err;
    err = auth_transcript_append_point (&t, (const uint8_t *)"device_pub", 10, device_pub);                 if (err) return err;
    err = auth_transcript_append_point (&t, (const uint8_t *)"server_pub", 10, server_pub);                 if (err) return err;
    err = auth_transcript_append_point (&t, (const uint8_t *)"a",          1,  a);                          if (err) return err;
    err = auth_transcript_append       (&t, (const uint8_t *)"client_nonce", 12, client_nonce, 32);         if (err) return err;
    err = auth_transcript_append       (&t, (const uint8_t *)"server_nonce", 12, server_nonce, 32);         if (err) return err;
    err = auth_transcript_append       (&t, (const uint8_t *)"setup_challenge", 15,
                                             setup_challenge_in, AUTH_SETUP_CHALLENGE_LEN);                 if (err) return err;
    return auth_transcript_challenge(&t, c_out);
}

auth_err_t auth_prove_setup_server_with_bytes(
    auth_schnorr_proof_t *proof,
    const uint8_t wide_r[64],
    const uint8_t server_sk[32],
    const uint8_t device_id[32],
    const uint8_t device_pub[32],
    const uint8_t server_pub[32],
    const uint8_t client_nonce[32],
    const uint8_t server_nonce[32],
    const uint8_t setup_challenge_in[AUTH_SETUP_CHALLENGE_LEN])
{
    uint8_t r[32];
    auth_scalar_from_wide(wide_r, r);

    auth_err_t err = g_times(proof->a, r);
    if (err) { sodium_memzero(r, sizeof r); return err; }

    uint8_t c[32];
    err = setup_server_challenge(c, device_id, device_pub, server_pub,
                                 proof->a, client_nonce, server_nonce,
                                 setup_challenge_in);
    if (err) { sodium_memzero(r, sizeof r); return err; }

    uint8_t cx[32];
    auth_scalar_mul(cx, c, server_sk);
    auth_scalar_add(proof->s, r, cx);

    sodium_memzero(r,  sizeof r);
    sodium_memzero(cx, sizeof cx);
    sodium_memzero(c,  sizeof c);
    return AUTH_OK;
}

auth_err_t auth_prove_setup_server(
    auth_schnorr_proof_t *proof,
    const uint8_t server_sk[32],
    const uint8_t device_id[32],
    const uint8_t device_pub[32],
    const uint8_t server_pub[32],
    const uint8_t client_nonce[32],
    const uint8_t server_nonce[32],
    const uint8_t setup_challenge_in[AUTH_SETUP_CHALLENGE_LEN])
{
    uint8_t wide[64];
    randombytes_buf(wide, sizeof wide);
    auth_err_t err = auth_prove_setup_server_with_bytes(
        proof, wide, server_sk, device_id, device_pub, server_pub,
        client_nonce, server_nonce, setup_challenge_in);
    sodium_memzero(wide, sizeof wide);
    return err;
}

int auth_verify_setup_server(
    const auth_schnorr_proof_t *proof,
    const uint8_t server_pub[32],
    const uint8_t device_id[32],
    const uint8_t device_pub[32],
    const uint8_t client_nonce[32],
    const uint8_t server_nonce[32],
    const uint8_t setup_challenge_in[AUTH_SETUP_CHALLENGE_LEN])
{
    uint8_t c[32];
    if (setup_server_challenge(c, device_id, device_pub, server_pub,
                               proof->a, client_nonce, server_nonce,
                               setup_challenge_in) != AUTH_OK) return 0;
    uint8_t gs[32], pc[32], rhs[32];
    if (g_times(gs, proof->s) != AUTH_OK) return 0;
    if (p_times(pc, c, server_pub) != AUTH_OK) return 0;
    if (auth_point_add(rhs, proof->a, pc) != AUTH_OK) return 0;
    return points_equal(gs, rhs);
}

/* ---- Auth-client Schnorr proof (bound to pid) ---- */

static auth_err_t auth_client_challenge(
    uint8_t c_out[32],
    const uint8_t pubkey[32],
    const uint8_t a[32],
    const uint8_t pid[32],
    const uint8_t nonce_c[32],
    const uint8_t eph_c[32])
{
    size_t dom_len;
    const uint8_t *dom = auth_ds_client_v2(&dom_len);

    auth_transcript_t t;
    auth_err_t err = auth_transcript_init(&t, dom, dom_len);
    if (err) return err;
    err = auth_transcript_append       (&t, (const uint8_t *)"pid",     3, pid,     32); if (err) return err;
    err = auth_transcript_append_point (&t, (const uint8_t *)"pubkey",  6, pubkey);      if (err) return err;
    err = auth_transcript_append_point (&t, (const uint8_t *)"a",       1, a);           if (err) return err;
    err = auth_transcript_append       (&t, (const uint8_t *)"nonce_c", 7, nonce_c, 32); if (err) return err;
    err = auth_transcript_append_point (&t, (const uint8_t *)"eph_c",   5, eph_c);       if (err) return err;
    return auth_transcript_challenge(&t, c_out);
}

auth_err_t auth_prove_auth_client_with_bytes(
    auth_schnorr_proof_t *proof,
    const uint8_t wide_r[64],
    const uint8_t x[32],
    const uint8_t pid[32],
    const uint8_t nonce_c[32],
    const uint8_t eph_c[32])
{
    uint8_t r[32];
    auth_scalar_from_wide(wide_r, r);

    auth_err_t err = g_times(proof->a, r);
    if (err) { sodium_memzero(r, sizeof r); return err; }

    uint8_t pubkey[32];
    err = g_times(pubkey, x);
    if (err) { sodium_memzero(r, sizeof r); return err; }

    uint8_t c[32];
    err = auth_client_challenge(c, pubkey, proof->a, pid, nonce_c, eph_c);
    if (err) { sodium_memzero(r, sizeof r); return err; }

    uint8_t cx[32];
    auth_scalar_mul(cx, c, x);
    auth_scalar_add(proof->s, r, cx);

    sodium_memzero(r,  sizeof r);
    sodium_memzero(cx, sizeof cx);
    sodium_memzero(c,  sizeof c);
    return AUTH_OK;
}

auth_err_t auth_prove_auth_client(
    auth_schnorr_proof_t *proof,
    const uint8_t x[32],
    const uint8_t pid[32],
    const uint8_t nonce_c[32],
    const uint8_t eph_c[32])
{
    uint8_t wide[64];
    randombytes_buf(wide, sizeof wide);
    auth_err_t err = auth_prove_auth_client_with_bytes(
        proof, wide, x, pid, nonce_c, eph_c);
    sodium_memzero(wide, sizeof wide);
    return err;
}

int auth_verify_auth_client(
    const auth_schnorr_proof_t *proof,
    const uint8_t device_pub[32],
    const uint8_t pid[32],
    const uint8_t nonce_c[32],
    const uint8_t eph_c[32])
{
    uint8_t c[32];
    if (auth_client_challenge(c, device_pub, proof->a, pid, nonce_c, eph_c)
        != AUTH_OK) return 0;
    uint8_t gs[32], pc[32], rhs[32];
    if (g_times(gs, proof->s) != AUTH_OK) return 0;
    if (p_times(pc, c, device_pub) != AUTH_OK) return 0;
    if (auth_point_add(rhs, proof->a, pc) != AUTH_OK) return 0;
    return points_equal(gs, rhs);
}

/* ---- Auth-server Schnorr proof ---- */

static auth_err_t auth_server_challenge(
    uint8_t c_out[32],
    const uint8_t server_pub[32],
    const uint8_t a[32],
    const uint8_t nonce_s[32],
    const uint8_t eph_s[32])
{
    size_t dom_len;
    const uint8_t *dom = auth_ds_server(&dom_len);

    auth_transcript_t t;
    auth_err_t err = auth_transcript_init(&t, dom, dom_len);
    if (err) return err;
    err = auth_transcript_append_point(&t, (const uint8_t *)"pubkey",  6, server_pub); if (err) return err;
    err = auth_transcript_append_point(&t, (const uint8_t *)"a",       1, a);          if (err) return err;
    err = auth_transcript_append      (&t, (const uint8_t *)"nonce_s", 7, nonce_s, 32); if (err) return err;
    err = auth_transcript_append_point(&t, (const uint8_t *)"eph_s",   5, eph_s);      if (err) return err;
    return auth_transcript_challenge(&t, c_out);
}

auth_err_t auth_prove_auth_server_with_bytes(
    auth_schnorr_proof_t *proof,
    const uint8_t wide_r[64],
    const uint8_t server_sk[32],
    const uint8_t nonce_s[32],
    const uint8_t eph_s[32])
{
    uint8_t r[32];
    auth_scalar_from_wide(wide_r, r);

    auth_err_t err = g_times(proof->a, r);
    if (err) { sodium_memzero(r, sizeof r); return err; }

    uint8_t server_pub[32];
    err = g_times(server_pub, server_sk);
    if (err) { sodium_memzero(r, sizeof r); return err; }

    uint8_t c[32];
    err = auth_server_challenge(c, server_pub, proof->a, nonce_s, eph_s);
    if (err) { sodium_memzero(r, sizeof r); return err; }

    uint8_t cx[32];
    auth_scalar_mul(cx, c, server_sk);
    auth_scalar_add(proof->s, r, cx);

    sodium_memzero(r,  sizeof r);
    sodium_memzero(cx, sizeof cx);
    sodium_memzero(c,  sizeof c);
    return AUTH_OK;
}

auth_err_t auth_prove_auth_server(
    auth_schnorr_proof_t *proof,
    const uint8_t server_sk[32],
    const uint8_t nonce_s[32],
    const uint8_t eph_s[32])
{
    uint8_t wide[64];
    randombytes_buf(wide, sizeof wide);
    auth_err_t err = auth_prove_auth_server_with_bytes(
        proof, wide, server_sk, nonce_s, eph_s);
    sodium_memzero(wide, sizeof wide);
    return err;
}

int auth_verify_auth_server(
    const auth_schnorr_proof_t *proof,
    const uint8_t server_pub[32],
    const uint8_t nonce_s[32],
    const uint8_t eph_s[32])
{
    uint8_t c[32];
    if (auth_server_challenge(c, server_pub, proof->a, nonce_s, eph_s)
        != AUTH_OK) return 0;
    uint8_t gs[32], pc[32], rhs[32];
    if (g_times(gs, proof->s) != AUTH_OK) return 0;
    if (p_times(pc, c, server_pub) != AUTH_OK) return 0;
    if (auth_point_add(rhs, proof->a, pc) != AUTH_OK) return 0;
    return points_equal(gs, rhs);
}

/* ---- Role commitment helpers ---- */

auth_err_t auth_make_role_commitment(
    uint8_t out[32],
    uint64_t role_code,
    const uint8_t blind[32])
{
    uint8_t role_s[32], h[32], g_role[32], h_blind[32];
    auth_scalar_from_u64(role_s, role_code);

    auth_err_t err = auth_attr_h(h);
    if (err) return err;

    err = auth_scalarmult_base(g_role, role_s);
    if (err) return err;

    err = auth_scalarmult(h_blind, blind, h);
    if (err) return err;

    return auth_point_add(out, g_role, h_blind);
}

auth_err_t auth_rerandomize_commitment(
    uint8_t c_prime_out[32],
    uint8_t blind_prime_out[32],
    const uint8_t stored_c[32],
    const uint8_t stored_blind[32],
    const uint8_t delta[32])
{
    uint8_t h[32], h_delta[32];
    auth_err_t err = auth_attr_h(h);
    if (err) return err;
    err = auth_scalarmult(h_delta, delta, h);
    if (err) return err;
    err = auth_point_add(c_prime_out, stored_c, h_delta);
    if (err) return err;
    auth_scalar_add(blind_prime_out, stored_blind, delta);
    return AUTH_OK;
}

/* ---- Rerandomization proof ---- */

static auth_err_t rerand_challenge(
    uint8_t c_out[32],
    const uint8_t pid[32],
    const uint8_t nonce_c[32],
    const uint8_t eph_c[32],
    const uint8_t stored_c[32],
    const uint8_t c_prime[32],
    const uint8_t a[32])
{
    size_t dom_len;
    const uint8_t *dom = auth_ds_role_rerand(&dom_len);

    auth_transcript_t t;
    auth_err_t err = auth_transcript_init(&t, dom, dom_len);
    if (err) return err;
    err = auth_transcript_append      (&t, (const uint8_t *)"pid",      3, pid,     32); if (err) return err;
    err = auth_transcript_append      (&t, (const uint8_t *)"nonce_c",  7, nonce_c, 32); if (err) return err;
    err = auth_transcript_append_point(&t, (const uint8_t *)"eph_c",    5, eph_c);       if (err) return err;
    err = auth_transcript_append_point(&t, (const uint8_t *)"stored_c", 8, stored_c);    if (err) return err;
    err = auth_transcript_append_point(&t, (const uint8_t *)"c_prime",  7, c_prime);     if (err) return err;
    err = auth_transcript_append_point(&t, (const uint8_t *)"a",        1, a);           if (err) return err;
    return auth_transcript_challenge(&t, c_out);
}

auth_err_t auth_prove_rerandomization_with_bytes(
    auth_schnorr_proof_t *proof,
    const uint8_t wide_r[64],
    const uint8_t stored_c[32],
    const uint8_t c_prime[32],
    const uint8_t delta[32],
    const uint8_t pid[32],
    const uint8_t nonce_c[32],
    const uint8_t eph_c[32])
{
    uint8_t h[32];
    auth_err_t err = auth_attr_h(h);
    if (err) return err;

    uint8_t r[32];
    auth_scalar_from_wide(wide_r, r);

    /* a = h * r */
    err = auth_scalarmult(proof->a, r, h);
    if (err) { sodium_memzero(r, sizeof r); return err; }

    uint8_t c[32];
    err = rerand_challenge(c, pid, nonce_c, eph_c, stored_c, c_prime, proof->a);
    if (err) { sodium_memzero(r, sizeof r); return err; }

    /* s = r + c * delta */
    uint8_t cd[32];
    auth_scalar_mul(cd, c, delta);
    auth_scalar_add(proof->s, r, cd);

    sodium_memzero(r,  sizeof r);
    sodium_memzero(cd, sizeof cd);
    sodium_memzero(c,  sizeof c);
    return AUTH_OK;
}

auth_err_t auth_prove_rerandomization(
    auth_schnorr_proof_t *proof,
    const uint8_t stored_c[32],
    const uint8_t c_prime[32],
    const uint8_t delta[32],
    const uint8_t pid[32],
    const uint8_t nonce_c[32],
    const uint8_t eph_c[32])
{
    uint8_t wide[64];
    randombytes_buf(wide, sizeof wide);
    auth_err_t err = auth_prove_rerandomization_with_bytes(
        proof, wide, stored_c, c_prime, delta, pid, nonce_c, eph_c);
    sodium_memzero(wide, sizeof wide);
    return err;
}

int auth_verify_rerandomization(
    const auth_schnorr_proof_t *proof,
    const uint8_t stored_c[32],
    const uint8_t c_prime[32],
    const uint8_t pid[32],
    const uint8_t nonce_c[32],
    const uint8_t eph_c[32])
{
    uint8_t h[32];
    if (auth_attr_h(h) != AUTH_OK) return 0;

    uint8_t c[32];
    if (rerand_challenge(c, pid, nonce_c, eph_c, stored_c, c_prime, proof->a)
        != AUTH_OK) return 0;

    /* Check h*s == a + (c_prime - stored_c)*c */
    uint8_t hs[32], diff[32], diff_c[32], rhs[32];
    if (auth_scalarmult(hs, proof->s, h) != AUTH_OK) return 0;
    if (auth_point_sub(diff, c_prime, stored_c) != AUTH_OK) return 0;
    if (auth_scalarmult(diff_c, c, diff) != AUTH_OK) return 0;
    if (auth_point_add(rhs, proof->a, diff_c) != AUTH_OK) return 0;
    return points_equal(hs, rhs);
}

/* ---- CDS-OR role-set-membership proof ---- */
/*
 * Let Y_i = C' - g*r_i. Provider knows delta such that for some k,
 * Y_k = h * blind_prime. The CDS-OR proof commits A_i for every i,
 * simulates (c_i, s_i) for every i != k, derives the master challenge
 * c = H(transcript(pid, nonce_c, eph_c, c_prime, r_0..r_{n-1}, A_0..A_{n-1}))
 * sets c_k = c - sum(c_i for i != k), s_k = w_true + c_k * blind_prime,
 * and publishes (A_i, c_i, s_i) for every i.
 *
 * Verifier checks:
 *    sum(c_i) == c  (mod L)
 *    for each i: h*s_i == A_i + Y_i*c_i
 */

static auth_err_t role_set_master_challenge(
    uint8_t master_c_out[32],
    const uint64_t *roles, size_t n,
    const uint8_t c_prime[32],
    const uint8_t *A_points /* 32*n bytes */,
    const uint8_t pid[32],
    const uint8_t nonce_c[32],
    const uint8_t eph_c[32])
{
    size_t dom_len;
    const uint8_t *dom = auth_ds_role_set(&dom_len);

    auth_transcript_t t;
    auth_err_t err = auth_transcript_init(&t, dom, dom_len);
    if (err) return err;
    err = auth_transcript_append      (&t, (const uint8_t *)"pid",     3, pid,     32); if (err) return err;
    err = auth_transcript_append      (&t, (const uint8_t *)"nonce_c", 7, nonce_c, 32); if (err) return err;
    err = auth_transcript_append_point(&t, (const uint8_t *)"eph_c",   5, eph_c);       if (err) return err;
    err = auth_transcript_append_point(&t, (const uint8_t *)"c_prime", 7, c_prime);     if (err) return err;
    for (size_t i = 0; i < n; ++i) {
        char label[16];
        int ll = snprintf(label, sizeof label, "r_%zu", i);
        if (ll < 0 || (size_t)ll >= sizeof label) return AUTH_ERR_INVALID_ARGUMENT;
        err = auth_transcript_append_u64(&t, (const uint8_t *)label, (size_t)ll, roles[i]);
        if (err) return err;
    }
    for (size_t i = 0; i < n; ++i) {
        char label[16];
        int ll = snprintf(label, sizeof label, "A_%zu", i);
        if (ll < 0 || (size_t)ll >= sizeof label) return AUTH_ERR_INVALID_ARGUMENT;
        err = auth_transcript_append_point(&t, (const uint8_t *)label, (size_t)ll,
                                               A_points + 32 * i);
        if (err) return err;
    }
    return auth_transcript_challenge(&t, master_c_out);
}

auth_err_t auth_prove_role_set_membership_with_bytes(
    auth_set_branch_t *branches,
    size_t *n_branches_out,
    const uint8_t *wide_bytes, size_t wide_bytes_len,
    const uint64_t *roles, size_t n,
    const uint8_t c_prime[32],
    uint64_t role_code,
    const uint8_t blind_prime[32],
    const uint8_t pid[32],
    const uint8_t nonce_c[32],
    const uint8_t eph_c[32])
{
    if (!branches || !n_branches_out || !roles || n == 0 ||
        n > AUTH_MAX_ROLES) {
        return AUTH_ERR_INVALID_ARGUMENT;
    }

    /* Find true index. */
    size_t true_idx = n;
    for (size_t i = 0; i < n; ++i) {
        if (roles[i] == role_code) { true_idx = i; break; }
    }
    if (true_idx == n) return AUTH_ERR_ROLE_NOT_PERMITTED;

    /* Minimum bytes required: 64 for w_true + 2*64 for every other branch. */
    size_t required = 64 + 2 * 64 * (n - 1);
    if (wide_bytes_len < required) return AUTH_ERR_INVALID_ARGUMENT;
    size_t consumed = 0;

    uint8_t h[32];
    auth_err_t err = auth_attr_h(h);
    if (err) return err;

    /* Y_i = c_prime - g*r_i */
    uint8_t Y[AUTH_MAX_ROLES][32];
    for (size_t i = 0; i < n; ++i) {
        uint8_t r_s[32], gr[32];
        auth_scalar_from_u64(r_s, roles[i]);
        err = auth_scalarmult_base(gr, r_s);
        if (err) return err;
        err = auth_point_sub(Y[i], c_prime, gr);
        if (err) return err;
    }

    /* Build A_i and simulated (c_i, s_i). Matches Rust order exactly:
     *   for i in 0..n:
     *     if i == true_index: consume 64B for w, A_i = h*w
     *     else:               consume 64B for c_i, 64B for s_i,
     *                         A_i = h*s_i - Y_i*c_i
     */
    uint8_t w_true[32] = {0};
    uint8_t A_all[AUTH_MAX_ROLES * 32];
    memset(&branches[0], 0, sizeof(auth_set_branch_t) * n);

    for (size_t i = 0; i < n; ++i) {
        if (i == true_idx) {
            auth_scalar_from_wide(&wide_bytes[consumed], w_true);
            consumed += 64;
            err = auth_scalarmult(branches[i].a, w_true, h);
            if (err) { sodium_memzero(w_true, sizeof w_true); return err; }
        } else {
            uint8_t ci[32], si[32];
            auth_scalar_from_wide(&wide_bytes[consumed], ci);
            consumed += 64;
            auth_scalar_from_wide(&wide_bytes[consumed], si);
            consumed += 64;

            /* A_i = h*s_i - Y_i*c_i */
            uint8_t hs[32], yc[32];
            err = auth_scalarmult(hs, si, h);
            if (err) { sodium_memzero(w_true, sizeof w_true); return err; }
            err = auth_scalarmult(yc, ci, Y[i]);
            if (err) { sodium_memzero(w_true, sizeof w_true); return err; }
            err = auth_point_sub(branches[i].a, hs, yc);
            if (err) { sodium_memzero(w_true, sizeof w_true); return err; }

            memcpy(branches[i].c, ci, 32);
            memcpy(branches[i].s, si, 32);
            sodium_memzero(ci, sizeof ci);
            sodium_memzero(si, sizeof si);
        }
        memcpy(&A_all[32 * i], branches[i].a, 32);
    }

    uint8_t master_c[32];
    err = role_set_master_challenge(master_c, roles, n, c_prime, A_all,
                                    pid, nonce_c, eph_c);
    if (err) { sodium_memzero(w_true, sizeof w_true); return err; }

    /* c_true = master - sum(c_i for i != true) */
    uint8_t sum_sim[32] = {0};
    for (size_t i = 0; i < n; ++i) {
        if (i != true_idx) {
            uint8_t tmp[32];
            auth_scalar_add(tmp, sum_sim, branches[i].c);
            memcpy(sum_sim, tmp, 32);
        }
    }
    uint8_t c_true[32];
    auth_scalar_sub(c_true, master_c, sum_sim);

    /* s_true = w_true + c_true * blind_prime */
    uint8_t cb[32];
    auth_scalar_mul(cb, c_true, blind_prime);
    auth_scalar_add(branches[true_idx].s, w_true, cb);
    memcpy(branches[true_idx].c, c_true, 32);

    sodium_memzero(w_true,  sizeof w_true);
    sodium_memzero(cb,      sizeof cb);
    sodium_memzero(c_true,  sizeof c_true);
    sodium_memzero(master_c,sizeof master_c);
    sodium_memzero(sum_sim, sizeof sum_sim);

    *n_branches_out = n;
    return AUTH_OK;
}

auth_err_t auth_prove_role_set_membership(
    auth_set_branch_t *branches,
    size_t *n_branches_out,
    const uint64_t *roles, size_t n,
    const uint8_t c_prime[32],
    uint64_t role_code,
    const uint8_t blind_prime[32],
    const uint8_t pid[32],
    const uint8_t nonce_c[32],
    const uint8_t eph_c[32])
{
    if (n == 0 || n > AUTH_MAX_ROLES) return AUTH_ERR_INVALID_ARGUMENT;
    size_t needed = 64 + 2 * 64 * (n - 1);
    uint8_t buf[64 + 2 * 64 * AUTH_MAX_ROLES];
    randombytes_buf(buf, needed);
    auth_err_t err = auth_prove_role_set_membership_with_bytes(
        branches, n_branches_out, buf, needed, roles, n, c_prime,
        role_code, blind_prime, pid, nonce_c, eph_c);
    sodium_memzero(buf, needed);
    return err;
}

int auth_verify_role_set_membership(
    const auth_set_branch_t *branches, size_t n_branches,
    const uint64_t *roles, size_t n_roles,
    const uint8_t c_prime[32],
    const uint8_t pid[32],
    const uint8_t nonce_c[32],
    const uint8_t eph_c[32])
{
    if (!branches || !roles || n_branches != n_roles ||
        n_roles == 0 || n_roles > AUTH_MAX_ROLES) return 0;

    uint8_t h[32];
    if (auth_attr_h(h) != AUTH_OK) return 0;

    /* Y_i = c_prime - g*r_i */
    uint8_t Y[AUTH_MAX_ROLES][32];
    uint8_t A_all[AUTH_MAX_ROLES * 32];
    for (size_t i = 0; i < n_roles; ++i) {
        uint8_t r_s[32], gr[32];
        auth_scalar_from_u64(r_s, roles[i]);
        if (auth_scalarmult_base(gr, r_s)      != AUTH_OK) return 0;
        if (auth_point_sub(Y[i], c_prime, gr)  != AUTH_OK) return 0;
        memcpy(&A_all[32 * i], branches[i].a, 32);
    }

    uint8_t master_c[32];
    if (role_set_master_challenge(master_c, roles, n_roles, c_prime, A_all,
                                  pid, nonce_c, eph_c) != AUTH_OK) return 0;

    /* sum(c_i) == master_c */
    uint8_t sum[32] = {0};
    for (size_t i = 0; i < n_roles; ++i) {
        uint8_t tmp[32];
        auth_scalar_add(tmp, sum, branches[i].c);
        memcpy(sum, tmp, 32);
    }
    if (!points_equal(sum, master_c)) return 0;  /* not points but same-size constant-time eq */

    /* For each i: h*s_i == A_i + Y_i*c_i */
    for (size_t i = 0; i < n_roles; ++i) {
        uint8_t hs[32], yc[32], rhs[32];
        if (auth_scalarmult(hs, branches[i].s, h)      != AUTH_OK) return 0;
        if (auth_scalarmult(yc, branches[i].c, Y[i])   != AUTH_OK) return 0;
        if (auth_point_add(rhs, branches[i].a, yc)     != AUTH_OK) return 0;
        if (!points_equal(hs, rhs)) return 0;
    }
    return 1;
}

/* ---- Session key and KC ---- */

auth_err_t auth_derive_session_key(
    uint8_t session_key_out[AUTH_SESSION_KEY_LEN],
    const uint8_t eph_secret[32],
    const uint8_t peer_eph_pub[32],
    const uint8_t nonce_c[32],
    const uint8_t nonce_s[32],
    const uint8_t pid[32],
    const uint8_t eph_c[32],
    const uint8_t eph_s[32])
{
    uint8_t shared[32];
    auth_err_t err = auth_scalarmult(shared, eph_secret, peer_eph_pub);
    if (err) return err;

    uint8_t salt[64];
    memcpy(salt,     nonce_c, 32);
    memcpy(salt + 32, nonce_s, 32);

    uint8_t info[14 + 32 + 32 + 32];
    memcpy(info,          "session key v2", 14);
    memcpy(info + 14,      pid,    32);
    memcpy(info + 14 + 32, eph_c,  32);
    memcpy(info + 14 + 64, eph_s,  32);

    err = auth_hkdf_sha256(session_key_out, AUTH_SESSION_KEY_LEN,
                               salt, sizeof salt,
                               shared, 32,
                               info, sizeof info);
    sodium_memzero(shared, sizeof shared);
    return err;
}

auth_err_t auth_kc_transcript_hash(
    uint8_t th_out[AUTH_HASH_LEN],
    const uint8_t pid[32],
    const auth_schnorr_proof_t *cp,
    const uint8_t nonce_c[32],
    const uint8_t eph_c[32],
    const uint8_t server_pub[32],
    const auth_schnorr_proof_t *sp,
    const uint8_t nonce_s[32],
    const uint8_t eph_s[32])
{
    size_t dom_len;
    const uint8_t *dom = auth_ds_kc_v2(&dom_len);

    auth_transcript_t t;
    auth_err_t err = auth_transcript_init(&t, dom, dom_len);
    if (err) return err;
    err = auth_transcript_append       (&t, (const uint8_t *)"pid",        3, pid, 32);       if (err) return err;
    err = auth_transcript_append_point (&t, (const uint8_t *)"a_c",        3, cp->a);         if (err) return err;
    err = auth_transcript_append_scalar(&t, (const uint8_t *)"s_c",        3, cp->s);         if (err) return err;
    err = auth_transcript_append       (&t, (const uint8_t *)"nonce_c",    7, nonce_c, 32);   if (err) return err;
    err = auth_transcript_append_point (&t, (const uint8_t *)"eph_c",      5, eph_c);         if (err) return err;
    err = auth_transcript_append_point (&t, (const uint8_t *)"server_pub", 10, server_pub);   if (err) return err;
    err = auth_transcript_append_point (&t, (const uint8_t *)"a_s",        3, sp->a);         if (err) return err;
    err = auth_transcript_append_scalar(&t, (const uint8_t *)"s_s",        3, sp->s);         if (err) return err;
    err = auth_transcript_append       (&t, (const uint8_t *)"nonce_s",    7, nonce_s, 32);   if (err) return err;
    err = auth_transcript_append_point (&t, (const uint8_t *)"eph_s",      5, eph_s);         if (err) return err;
    return auth_transcript_hash_sha256(&t, th_out);
}

void auth_derive_kc_keys(
    uint8_t k_s2c_out[AUTH_MAC_KEY_LEN],
    uint8_t k_c2s_out[AUTH_MAC_KEY_LEN],
    const uint8_t session_key[AUTH_SESSION_KEY_LEN],
    const uint8_t th[AUTH_HASH_LEN])
{
    /* HKDF-SHA256(salt=th, ikm=session_key, info=<label>) for each direction */
    (void)auth_hkdf_sha256(k_s2c_out, AUTH_MAC_KEY_LEN,
                               th, AUTH_HASH_LEN,
                               session_key, AUTH_SESSION_KEY_LEN,
                               (const uint8_t *)"kc s2c", 6);
    (void)auth_hkdf_sha256(k_c2s_out, AUTH_MAC_KEY_LEN,
                               th, AUTH_HASH_LEN,
                               session_key, AUTH_SESSION_KEY_LEN,
                               (const uint8_t *)"kc c2s", 6);
}
