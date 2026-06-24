# DRBG specification for test vectors (suite 0x0001)

All probabilistic fields in these vectors are derived from a single
deterministic bit generator so that any conforming implementation can
reproduce them byte-for-byte.

## Seed

```
DRBG_SEED = b"iot-auth/test-vectors/v1        "  (exactly 32 bytes)
```

## Generator

ChaCha20 with the above seed as the 256-bit key, 96-bit nonce = all zeros,
block counter starts at 0. Output is the keystream. This matches the
`rand_chacha::ChaCha20Rng::from_seed` RFC-compatible mode.

## Scalar sampling

```
rand_scalar(rng):
    bytes ← rng.next_bytes(64)          # 64 bytes = 512 bits
    return Scalar::from_bytes_mod_order_wide(bytes)
```

i.e. 64 uniform bytes, reduced mod the ristretto255 scalar group order via
the standard "wide" reduction used by libsodium, ed25519-donna, and
curve25519-dalek (all of these produce identical output given identical
64-byte inputs).

## Consumption order

Each vector file uses a **fresh** generator seeded with `DRBG_SEED`. The
bytes consumed inside a single vector are listed in the `inputs` block.
For provers that call `rand_scalar` multiple times, the consumption order
is:

- `schnorr_auth_client.json`:    64 bytes for `r`.
- `rerandomization.json`:        64 bytes for `r`.
- `role_set_membership.json`:    for each branch in `0..n`, if it is the
                                 true index consume 64 bytes (`w`), else
                                 consume 128 bytes (`c_i`, `s_i`).
- `kdf_kc.json`:                 bytes 0..64 → client Schnorr `r`;
                                 bytes 64..128 → server Schnorr `r`.

A reproducing implementation that uses the same seed, the same generator,
and consumes bytes in the same order will obtain the same `(a, s)` for
every proof published in this corpus.
