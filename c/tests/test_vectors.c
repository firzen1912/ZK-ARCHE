/*
 * test_vectors.c — cross-language conformance against the Rust
 * reference fixtures under test-vectors/0x0001/.
 *
 * Validates all six JSON vectors byte-for-byte:
 *
 *   transcript.json           canonical byte layout + challenge scalar
 *   pid.json                  PID derivation
 *   schnorr_auth_client.json  deterministic Schnorr prove + verify
 *   rerandomization.json      deterministic rerandomization proof
 *   role_set_membership.json  deterministic CDS-OR set-membership proof
 *   kdf_kc.json               HKDF session key + KC transcript hash + tags
 *
 * DRBG: ChaCha20 IETF with 32-byte seed key and zero 12-byte nonce.
 * See docs/assurance-and-validation.md for the full specification.
 */

#include "auth/iot_auth.h"
#include "auth/auth_crypto.h"
#include "auth/auth_proofs.h"
#include "auth/auth_transcript.h"

#include <sodium.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- Hex utility ---- */

static int hex_to_bytes(const char *hex, uint8_t *out, size_t out_len)
{
    size_t hlen = strlen(hex);
    if (hlen != out_len * 2) return -1;
    for (size_t i = 0; i < out_len; ++i) {
        unsigned int byte;
        if (sscanf(hex + 2 * i, "%2x", &byte) != 1) return -1;
        out[i] = (uint8_t)byte;
    }
    return 0;
}

static void print_hex(const char *label, const uint8_t *b, size_t n)
{
    printf("  %s: ", label);
    for (size_t i = 0; i < n; ++i) printf("%02x", b[i]);
    printf("\n");
}

static int fail(const char *msg) { printf("  FAIL: %s\n", msg); return 1; }

/* ---- Minimal JSON string-field extractor. Does not handle nested quotes. ---- */

static int find_string_field(const char *json, const char *key,
                             char *out, size_t out_cap)
{
    char pattern[128];
    int n = snprintf(pattern, sizeof pattern, "\"%s\"", key);
    if (n <= 0 || (size_t)n >= sizeof pattern) return -1;
    const char *p = strstr(json, pattern);
    if (!p) return -1;
    p += n;
    while (*p && (*p == ' ' || *p == ':' || *p == '\t' || *p == '\n')) p++;
    if (*p != '"') return -1;
    p++;
    size_t wrote = 0;
    while (*p && *p != '"' && wrote + 1 < out_cap) out[wrote++] = *p++;
    if (*p != '"') return -1;
    out[wrote] = 0;
    return (int)wrote;
}

static char *read_file(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    size_t nread = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    buf[nread] = 0;
    return buf;
}

/* ---- DRBG: ChaCha20-IETF keystream with fixed seed ----
 *
 * Rust rand_chacha::ChaCha20Rng::from_seed(seed) produces a stream that
 * is identical to RFC 8439 ChaCha20 with:
 *   key   = seed (32 bytes)
 *   nonce = 00 00 00 00 00 00 00 00 00 00 00 00
 *   counter starts at 0
 * That is exactly libsodium's crypto_stream_chacha20_ietf.
 */

static const uint8_t DRBG_SEED[32] = {
    'i','o','t','-','a','u','t','h','/',
    't','e','s','t','-','v','e','c','t','o','r','s','/',
    'v','1',' ',' ',' ',' ',' ',' ',' ',' '
};

/* Fill `buf` with the next `n` bytes from the stream starting at `offset`. */
static int drbg_generate(uint8_t *buf, size_t n, size_t offset)
{
    /* We do it simply: generate the first (offset + n) bytes and memcpy
     * the tail. For the tests this is well under a megabyte. */
    size_t total = offset + n;
    uint8_t *tmp = (uint8_t *)malloc(total);
    if (!tmp) return -1;
    uint8_t nonce[12] = {0};
    if (crypto_stream_chacha20_ietf(tmp, total, nonce, DRBG_SEED) != 0) {
        free(tmp); return -1;
    }
    memcpy(buf, tmp + offset, n);
    sodium_memzero(tmp, total);
    free(tmp);
    return 0;
}

/* ---- Vector 1: transcript ---- */

static int test_transcript(const char *path)
{
    printf("== transcript.json ==\n");
    char *json = read_file(path);
    if (!json) { printf("  SKIP\n"); return 0; }

    char exp_bytes_hex[1024], exp_scalar_hex[128];
    if (find_string_field(json, "transcript_bytes", exp_bytes_hex, sizeof exp_bytes_hex) < 0 ||
        find_string_field(json, "challenge_scalar", exp_scalar_hex, sizeof exp_scalar_hex) < 0) {
        free(json); return fail("missing field");
    }
    free(json);

    auth_transcript_t t;
    auth_transcript_init(&t, (const uint8_t *)"test-domain", 11);
    auth_transcript_append(&t, (const uint8_t *)"a", 1, (const uint8_t *)"hello", 5);
    static const uint8_t B[] = { 0x11, 0x22, 0x33, 0x44 };
    auth_transcript_append(&t, (const uint8_t *)"b", 1, B, 4);

    size_t exp_len = strlen(exp_bytes_hex) / 2;
    uint8_t exp_bytes[512];
    if (exp_len > sizeof exp_bytes) return fail("transcript too long");
    hex_to_bytes(exp_bytes_hex, exp_bytes, exp_len);
    if (t.len != exp_len || memcmp(t.buf, exp_bytes, exp_len) != 0) {
        print_hex("got", t.buf, t.len);
        print_hex("exp", exp_bytes, exp_len);
        return fail("transcript bytes mismatch");
    }
    printf("  transcript bytes: MATCH (%zu bytes)\n", exp_len);

    uint8_t got[32], exp[32];
    auth_transcript_challenge(&t, got);
    hex_to_bytes(exp_scalar_hex, exp, 32);
    if (memcmp(got, exp, 32) != 0) return fail("challenge mismatch");
    printf("  challenge scalar: MATCH\n");
    return 0;
}

/* ---- Vector 2: pid ---- */

static int test_pid(const char *path)
{
    printf("== pid.json ==\n");
    char *json = read_file(path);
    if (!json) { printf("  SKIP\n"); return 0; }

    char dp[65], nc[65], ec[65], sp[65], exp[65];
    if (find_string_field(json, "device_pub", dp, sizeof dp) < 0 ||
        find_string_field(json, "nonce_c",    nc, sizeof nc) < 0 ||
        find_string_field(json, "eph_c",      ec, sizeof ec) < 0 ||
        find_string_field(json, "server_pub", sp, sizeof sp) < 0 ||
        find_string_field(json, "pid",        exp, sizeof exp) < 0) {
        free(json); return fail("missing field");
    }
    free(json);

    uint8_t device_pub[32], nonce_c[32], eph_c[32], server_pub[32], exp_pid[32];
    hex_to_bytes(dp, device_pub, 32);
    hex_to_bytes(nc, nonce_c, 32);
    hex_to_bytes(ec, eph_c, 32);
    hex_to_bytes(sp, server_pub, 32);
    hex_to_bytes(exp, exp_pid, 32);

    uint8_t got[32];
    auth_compute_pid(got, device_pub, nonce_c, eph_c, server_pub);
    if (memcmp(got, exp_pid, 32) != 0) return fail("pid mismatch");
    printf("  pid: MATCH\n");
    return 0;
}

/* ---- Vector 3: schnorr_auth_client (DETERMINISTIC) ---- */

static int test_schnorr_auth(const char *path)
{
    printf("== schnorr_auth_client.json ==\n");
    char *json = read_file(path);
    if (!json) { printf("  SKIP\n"); return 0; }

    char x_hex[65], dp_hex[65], pid_hex[65], nc_hex[65], ec_hex[65];
    char drbg_hex[129], exp_a_hex[65], exp_s_hex[65];
    if (find_string_field(json, "x",                x_hex,   sizeof x_hex) < 0 ||
        find_string_field(json, "device_pub",       dp_hex,  sizeof dp_hex) < 0 ||
        find_string_field(json, "pid",              pid_hex, sizeof pid_hex) < 0 ||
        find_string_field(json, "nonce_c",          nc_hex,  sizeof nc_hex) < 0 ||
        find_string_field(json, "eph_c",            ec_hex,  sizeof ec_hex) < 0 ||
        find_string_field(json, "drbg_bytes_for_r", drbg_hex,sizeof drbg_hex) < 0 ||
        find_string_field(json, "proof_a",          exp_a_hex, sizeof exp_a_hex) < 0 ||
        find_string_field(json, "proof_s",          exp_s_hex, sizeof exp_s_hex) < 0) {
        free(json); return fail("missing field");
    }
    free(json);

    uint8_t x[32], device_pub[32], pid[32], nc[32], ec[32];
    uint8_t exp_drbg[64], exp_a[32], exp_s[32];
    hex_to_bytes(x_hex,       x,          32);
    hex_to_bytes(dp_hex,      device_pub, 32);
    hex_to_bytes(pid_hex,     pid,        32);
    hex_to_bytes(nc_hex,      nc,         32);
    hex_to_bytes(ec_hex,      ec,         32);
    hex_to_bytes(drbg_hex,    exp_drbg,   64);
    hex_to_bytes(exp_a_hex,   exp_a,      32);
    hex_to_bytes(exp_s_hex,   exp_s,      32);

    /* First: confirm our DRBG yields the expected first 64 bytes. */
    uint8_t drbg_got[64];
    if (drbg_generate(drbg_got, 64, 0) != 0) return fail("drbg failed");
    if (memcmp(drbg_got, exp_drbg, 64) != 0) {
        print_hex("drbg-got", drbg_got, 64);
        print_hex("drbg-exp", exp_drbg, 64);
        return fail("drbg mismatch (chacha20 incompatible?)");
    }
    printf("  drbg first 64 bytes: MATCH\n");

    /* Now prove with those bytes and compare. */
    auth_schnorr_proof_t proof;
    auth_err_t err = auth_prove_auth_client_with_bytes(
        &proof, drbg_got, x, pid, nc, ec);
    if (err) return fail("prove failed");

    if (memcmp(proof.a, exp_a, 32) != 0) {
        print_hex("got a", proof.a, 32);
        print_hex("exp a", exp_a, 32);
        return fail("proof_a mismatch");
    }
    if (memcmp(proof.s, exp_s, 32) != 0) {
        print_hex("got s", proof.s, 32);
        print_hex("exp s", exp_s, 32);
        return fail("proof_s mismatch");
    }
    printf("  proof (a, s): MATCH\n");

    /* Verify. */
    if (!auth_verify_auth_client(&proof, device_pub, pid, nc, ec)) {
        return fail("verify with correct pubkey failed");
    }
    printf("  verify_ok_own: MATCH\n");

    /* Verify must fail with wrong pubkey. */
    uint8_t wrong_sk[32], wrong_pub[32];
    auth_scalar_from_u64(wrong_sk, 3);
    auth_scalarmult_base(wrong_pub, wrong_sk);
    if (auth_verify_auth_client(&proof, wrong_pub, pid, nc, ec)) {
        return fail("verify with wrong pubkey unexpectedly succeeded");
    }
    printf("  verify_ok_wrong_pubkey: MATCH (rejected)\n");
    return 0;
}

/* ---- Vector 4: rerandomization (DETERMINISTIC) ---- */

static int test_rerandomization(const char *path)
{
    printf("== rerandomization.json ==\n");
    char *json = read_file(path);
    if (!json) { printf("  SKIP\n"); return 0; }

    char sc_hex[65], blind_hex[65], delta_hex[65];
    char pid_hex[65], nc_hex[65], ec_hex[65];
    char drbg_hex[129], cp_hex[65], a_hex[65], s_hex[65];

    if (find_string_field(json, "stored_c",          sc_hex,    sizeof sc_hex) < 0 ||
        find_string_field(json, "blind",             blind_hex, sizeof blind_hex) < 0 ||
        find_string_field(json, "delta",             delta_hex, sizeof delta_hex) < 0 ||
        find_string_field(json, "pid",               pid_hex,   sizeof pid_hex) < 0 ||
        find_string_field(json, "nonce_c",           nc_hex,    sizeof nc_hex) < 0 ||
        find_string_field(json, "eph_c",             ec_hex,    sizeof ec_hex) < 0 ||
        find_string_field(json, "drbg_bytes_for_r",  drbg_hex,  sizeof drbg_hex) < 0 ||
        find_string_field(json, "c_prime",           cp_hex,    sizeof cp_hex) < 0 ||
        find_string_field(json, "proof_a",           a_hex,     sizeof a_hex) < 0 ||
        find_string_field(json, "proof_s",           s_hex,     sizeof s_hex) < 0)
    {
        free(json); return fail("missing field");
    }
    free(json);

    uint8_t stored_c[32], blind[32], delta[32], pid[32], nc[32], ec[32];
    uint8_t exp_drbg[64], exp_cp[32], exp_a[32], exp_s[32];
    hex_to_bytes(sc_hex, stored_c, 32);
    hex_to_bytes(blind_hex, blind, 32);
    hex_to_bytes(delta_hex, delta, 32);
    hex_to_bytes(pid_hex, pid, 32);
    hex_to_bytes(nc_hex, nc, 32);
    hex_to_bytes(ec_hex, ec, 32);
    hex_to_bytes(drbg_hex, exp_drbg, 64);
    hex_to_bytes(cp_hex, exp_cp, 32);
    hex_to_bytes(a_hex, exp_a, 32);
    hex_to_bytes(s_hex, exp_s, 32);

    /* Reproduce c_prime = stored_c + h*delta. */
    uint8_t c_prime[32], blind_prime[32];
    if (auth_rerandomize_commitment(c_prime, blind_prime,
                                        stored_c, blind, delta) != AUTH_OK) {
        return fail("rerand compute failed");
    }
    if (memcmp(c_prime, exp_cp, 32) != 0) {
        print_hex("got c_prime", c_prime, 32);
        print_hex("exp c_prime", exp_cp, 32);
        return fail("c_prime mismatch");
    }
    printf("  c_prime: MATCH\n");

    /* Prove with the expected DRBG bytes. */
    uint8_t drbg_got[64];
    drbg_generate(drbg_got, 64, 0);
    if (memcmp(drbg_got, exp_drbg, 64) != 0) return fail("drbg mismatch");

    auth_schnorr_proof_t proof;
    auth_err_t err = auth_prove_rerandomization_with_bytes(
        &proof, drbg_got, stored_c, c_prime, delta, pid, nc, ec);
    if (err) return fail("prove failed");

    if (memcmp(proof.a, exp_a, 32) != 0) {
        print_hex("got a", proof.a, 32);
        print_hex("exp a", exp_a, 32);
        return fail("proof_a mismatch");
    }
    if (memcmp(proof.s, exp_s, 32) != 0) {
        print_hex("got s", proof.s, 32);
        print_hex("exp s", exp_s, 32);
        return fail("proof_s mismatch");
    }
    printf("  proof (a, s): MATCH\n");

    if (!auth_verify_rerandomization(&proof, stored_c, c_prime, pid, nc, ec)) {
        return fail("verify failed");
    }
    printf("  verify_ok: MATCH\n");
    return 0;
}

/* ---- Vector 5: role set-membership (DETERMINISTIC) ---- */

static int test_role_set(const char *path)
{
    printf("== role_set_membership.json ==\n");
    char *json = read_file(path);
    if (!json) { printf("  SKIP\n"); return 0; }

    /* Hardcoded inputs that match the Rust generator. The generator
     * uses:
     *   allowed = [1, 2, 3]
     *   role_code = 2 (true_index = 1)
     *   blind     = Scalar::from(97)
     *   delta     = Scalar::from(41)
     *   pid       = 0xCD * 32
     *   nonce_c   = 0xEF * 32
     *   eph_c     = g * 101
     */
    uint64_t roles[3] = { 1, 2, 3 };

    /* Extract published c_prime, blind_prime, pid, nonce_c, eph_c. */
    char bp_hex[65], cp_hex[65], pid_hex[65], nc_hex[65], ec_hex[65];
    if (find_string_field(json, "blind_prime", bp_hex, sizeof bp_hex) < 0 ||
        find_string_field(json, "c_prime",     cp_hex, sizeof cp_hex) < 0 ||
        find_string_field(json, "pid",         pid_hex, sizeof pid_hex) < 0 ||
        find_string_field(json, "nonce_c",     nc_hex,  sizeof nc_hex) < 0 ||
        find_string_field(json, "eph_c",       ec_hex,  sizeof ec_hex) < 0)
    {
        free(json); return fail("missing field");
    }

    /* Extract expected branches. JSON structure is a sequence of
     * {"a":"...","c":"...","s":"..."} objects. For the tiny parser
     * we have, we peel off three in order. */
    /* Find "branches": [ then scan forward for "a":"...", "c":"...", "s":"..." triples */
    const char *p = strstr(json, "\"branches\"");
    if (!p) { free(json); return fail("no branches"); }

    char br_a[3][65], br_c[3][65], br_s[3][65];
    for (size_t i = 0; i < 3; ++i) {
        const char *ka = strstr(p, "\"a\"");
        const char *kc = ka ? strstr(ka, "\"c\"") : NULL;
        const char *ks = kc ? strstr(kc, "\"s\"") : NULL;
        if (!ka || !kc || !ks) { free(json); return fail("branch field missing"); }
        if (find_string_field(ka, "a", br_a[i], sizeof br_a[i]) < 0 ||
            find_string_field(kc, "c", br_c[i], sizeof br_c[i]) < 0 ||
            find_string_field(ks, "s", br_s[i], sizeof br_s[i]) < 0) {
            free(json); return fail("branch parse");
        }
        p = ks + 1;
    }
    free(json);

    uint8_t blind_prime[32], c_prime[32], pid[32], nc[32], ec[32];
    hex_to_bytes(bp_hex, blind_prime, 32);
    hex_to_bytes(cp_hex, c_prime,     32);
    hex_to_bytes(pid_hex, pid, 32);
    hex_to_bytes(nc_hex,  nc,  32);
    hex_to_bytes(ec_hex,  ec,  32);

    uint8_t exp_a[3][32], exp_c[3][32], exp_s[3][32];
    for (size_t i = 0; i < 3; ++i) {
        hex_to_bytes(br_a[i], exp_a[i], 32);
        hex_to_bytes(br_c[i], exp_c[i], 32);
        hex_to_bytes(br_s[i], exp_s[i], 32);
    }

    /* DRBG consumption order for n=3, true_index=1:
     *   i=0 (simulated):  64 bytes for c_0, 64 bytes for s_0     [0..128]
     *   i=1 (true):       64 bytes for w                         [128..192]
     *   i=2 (simulated):  64 bytes for c_2, 64 bytes for s_2     [192..320]
     *
     * So we need 320 bytes. The buffer passed to
     * auth_prove_role_set_membership_with_bytes is consumed
     * sequentially in the same order. */
    size_t total = 64 + 2 * 64 * (3 - 1);  /* 320 */
    uint8_t *wide = (uint8_t *)malloc(total);
    if (!wide) return fail("oom");
    if (drbg_generate(wide, total, 0) != 0) { free(wide); return fail("drbg"); }

    auth_set_branch_t branches[3];
    size_t n_out = 0;
    auth_err_t err = auth_prove_role_set_membership_with_bytes(
        branches, &n_out, wide, total, roles, 3, c_prime,
        2 /* role_code */, blind_prime, pid, nc, ec);
    sodium_memzero(wide, total);
    free(wide);
    if (err) { printf("  prove err: %s (0x%04x)\n",
                      auth_strerror(err), (unsigned)err);
               return fail("prove failed"); }
    if (n_out != 3) return fail("wrong n_out");

    for (size_t i = 0; i < 3; ++i) {
        if (memcmp(branches[i].a, exp_a[i], 32) != 0) {
            printf("  branch %zu A: ", i);
            print_hex("got", branches[i].a, 32);
            print_hex("exp", exp_a[i], 32);
            return fail("branch A mismatch");
        }
        if (memcmp(branches[i].c, exp_c[i], 32) != 0) {
            printf("  branch %zu C: ", i);
            print_hex("got", branches[i].c, 32);
            print_hex("exp", exp_c[i], 32);
            return fail("branch C mismatch");
        }
        if (memcmp(branches[i].s, exp_s[i], 32) != 0) {
            printf("  branch %zu S: ", i);
            print_hex("got", branches[i].s, 32);
            print_hex("exp", exp_s[i], 32);
            return fail("branch S mismatch");
        }
    }
    printf("  branches (a, c, s) x 3: MATCH\n");

    if (!auth_verify_role_set_membership(branches, 3, roles, 3,
                                             c_prime, pid, nc, ec)) {
        return fail("verify failed");
    }
    printf("  verify_ok: MATCH\n");

    /* Tamper and ensure rejection. */
    uint8_t c_prime_bad[32];
    uint8_t h[32];
    auth_attr_h(h);
    uint8_t s999[32];
    auth_scalar_from_u64(s999, 999);
    uint8_t bump[32];
    auth_scalarmult(bump, s999, h);
    auth_point_add(c_prime_bad, c_prime, bump);
    if (auth_verify_role_set_membership(branches, 3, roles, 3,
                                            c_prime_bad, pid, nc, ec)) {
        return fail("verify_wrong_c_prime unexpectedly succeeded");
    }
    printf("  verify_wrong_c_prime: MATCH (rejected)\n");
    return 0;
}

/* ---- Vector 6: KDF / KC (existing test, extended) ---- */

static int test_kdf_kc(const char *path)
{
    printf("== kdf_kc.json ==\n");
    char *json = read_file(path);
    if (!json) { printf("  SKIP\n"); return 0; }

    char client_eph_sk_hex[65], nonce_c_hex[65], nonce_s_hex[65];
    char pid_hex[65], eph_c_hex[65], eph_s_hex[65], server_pub_hex[65];
    char ac_hex[65], sc_hex[65], as_hex[65], ss_hex[65];
    char exp_key_hex[65], exp_th_hex[65], exp_tag_s_hex[65], exp_tag_c_hex[65];

    if (find_string_field(json, "client_eph_sk", client_eph_sk_hex, sizeof client_eph_sk_hex) < 0 ||
        find_string_field(json, "nonce_c", nonce_c_hex, sizeof nonce_c_hex) < 0 ||
        find_string_field(json, "nonce_s", nonce_s_hex, sizeof nonce_s_hex) < 0 ||
        find_string_field(json, "pid",     pid_hex,     sizeof pid_hex) < 0 ||
        find_string_field(json, "eph_c",   eph_c_hex,   sizeof eph_c_hex) < 0 ||
        find_string_field(json, "eph_s",   eph_s_hex,   sizeof eph_s_hex) < 0 ||
        find_string_field(json, "server_pub", server_pub_hex, sizeof server_pub_hex) < 0 ||
        find_string_field(json, "a_c", ac_hex, sizeof ac_hex) < 0 ||
        find_string_field(json, "s_c", sc_hex, sizeof sc_hex) < 0 ||
        find_string_field(json, "a_s", as_hex, sizeof as_hex) < 0 ||
        find_string_field(json, "s_s", ss_hex, sizeof ss_hex) < 0 ||
        find_string_field(json, "session_key_from_client", exp_key_hex, sizeof exp_key_hex) < 0 ||
        find_string_field(json, "transcript_hash",         exp_th_hex,  sizeof exp_th_hex) < 0 ||
        find_string_field(json, "tag_s",                   exp_tag_s_hex, sizeof exp_tag_s_hex) < 0 ||
        find_string_field(json, "tag_c",                   exp_tag_c_hex, sizeof exp_tag_c_hex) < 0)
    {
        free(json); return fail("missing field");
    }
    free(json);

    uint8_t client_eph_sk[32], nonce_c[32], nonce_s[32], pid[32];
    uint8_t eph_c[32], eph_s[32], server_pub[32];
    uint8_t ac[32], sc[32], as_[32], ss[32];
    uint8_t exp_key[32], exp_th[32], exp_tag_s[32], exp_tag_c[32];
    hex_to_bytes(client_eph_sk_hex, client_eph_sk, 32);
    hex_to_bytes(nonce_c_hex, nonce_c, 32);
    hex_to_bytes(nonce_s_hex, nonce_s, 32);
    hex_to_bytes(pid_hex,     pid,     32);
    hex_to_bytes(eph_c_hex,   eph_c,   32);
    hex_to_bytes(eph_s_hex,   eph_s,   32);
    hex_to_bytes(server_pub_hex, server_pub, 32);
    hex_to_bytes(ac_hex, ac, 32);
    hex_to_bytes(sc_hex, sc, 32);
    hex_to_bytes(as_hex, as_, 32);
    hex_to_bytes(ss_hex, ss, 32);
    hex_to_bytes(exp_key_hex, exp_key, 32);
    hex_to_bytes(exp_th_hex,  exp_th,  32);
    hex_to_bytes(exp_tag_s_hex, exp_tag_s, 32);
    hex_to_bytes(exp_tag_c_hex, exp_tag_c, 32);

    /* Session key */
    uint8_t got_key[32];
    if (auth_derive_session_key(got_key, client_eph_sk, eph_s,
                                    nonce_c, nonce_s, pid, eph_c, eph_s)
        != AUTH_OK) return fail("derive_session_key");
    if (memcmp(got_key, exp_key, 32) != 0) {
        print_hex("got", got_key, 32);
        print_hex("exp", exp_key, 32);
        return fail("session_key mismatch");
    }
    printf("  session_key: MATCH\n");

    /* Transcript hash */
    auth_schnorr_proof_t cp, sp;
    memcpy(cp.a, ac, 32); memcpy(cp.s, sc, 32);
    memcpy(sp.a, as_,32); memcpy(sp.s, ss, 32);
    uint8_t got_th[32];
    if (auth_kc_transcript_hash(got_th, pid, &cp, nonce_c, eph_c,
                                    server_pub, &sp, nonce_s, eph_s)
        != AUTH_OK) return fail("kc_transcript_hash");
    if (memcmp(got_th, exp_th, 32) != 0) {
        print_hex("got", got_th, 32);
        print_hex("exp", exp_th, 32);
        return fail("transcript_hash mismatch");
    }
    printf("  transcript_hash: MATCH\n");

    /* KC keys + tags */
    uint8_t k_s2c[32], k_c2s[32];
    auth_derive_kc_keys(k_s2c, k_c2s, got_key, got_th);
    uint8_t got_tag_s[32], got_tag_c[32];
    auth_hmac_tag(got_tag_s, k_s2c, (const uint8_t *)"server finished", 15, got_th);
    auth_hmac_tag(got_tag_c, k_c2s, (const uint8_t *)"client finished", 15, got_th);
    if (memcmp(got_tag_s, exp_tag_s, 32) != 0) return fail("tag_s mismatch");
    if (memcmp(got_tag_c, exp_tag_c, 32) != 0) return fail("tag_c mismatch");
    printf("  tag_s, tag_c: MATCH\n");
    return 0;
}

/* ---- Main ---- */

int main(int argc, char **argv)
{
    const char *dir = (argc > 1)
        ? argv[1]
        : "../rust/test-vectors/0x0001";

    if (auth_init() != AUTH_OK) {
        fprintf(stderr, "auth_init failed\n"); return 1;
    }

    char path[512];
    int failures = 0;

    snprintf(path, sizeof path, "%s/transcript.json", dir);
    failures += test_transcript(path);

    snprintf(path, sizeof path, "%s/pid.json", dir);
    failures += test_pid(path);

    snprintf(path, sizeof path, "%s/schnorr_auth_client.json", dir);
    failures += test_schnorr_auth(path);

    snprintf(path, sizeof path, "%s/rerandomization.json", dir);
    failures += test_rerandomization(path);

    snprintf(path, sizeof path, "%s/role_set_membership.json", dir);
    failures += test_role_set(path);

    snprintf(path, sizeof path, "%s/kdf_kc.json", dir);
    failures += test_kdf_kc(path);

    printf("\n%s: %d failure(s)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
