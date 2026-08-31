# Checkpoint: lineage replacement attempt/confirmation binding

Date: 2026-08-30
Roadmap owner: zk213
Claim class: reproducible Rust/C semantic evidence only

## Change under review

This checkpoint adds a wire-neutral classifier requiring an authenticated lineage replacement confirmation to agree on one attempt identity, predecessor generation, successor, and authenticated context before bilateral confirmation can produce `CONVERGED`.

Affected surfaces:

- `spec/lineage-replace-attempt-binding-contract.md`
- Rust and C attempt classifiers
- shared `AB-01` through `AB-12` canonical corpus
- Rust and C corpus consumers

## Security review checklist

- Normal AUTH remains NO-LEARNING: unchanged.
- Authentication is not treated as authorization: both local and peer replacement authorization remain explicit facts.
- Trust mutation is not performed by this classifier.
- A stale/different attempt cannot converge through current confirmation traffic.
- Predecessor generation, successor, and authenticated context mismatches fail closed before confirmation is considered.
- Missing confirmation binding remains pending rather than silently accepted.
- No transport address is used as protocol identity.
- No wire message, suite, primitive, timeout, or collision winner is allocated.

## Canonical negative coverage

`AB-04` rejects a different/stale attempt identity. `AB-05` rejects a predecessor-generation mismatch. `AB-06` rejects a different successor. `AB-07` rejects a context mismatch. `AB-08` and `AB-09` reject unilateral authorization. `AB-10` and `AB-11` show that absent/stale confirmation binding cannot complete the attempt. `AB-12` keeps a stale attempt mismatch authoritative even when confirmation evidence is absent.

## Evidence boundary

The packet is IMPLEMENTED in Rust and C source/test surfaces. Local executable evidence in this run is limited to the strict-warning C classifier and canonical corpus. Rust execution, rustfmt, full C repository build/libsodium integration, sanitizers, static analysis, formal synchronization/ProVerif, release qualification, hardware measurements, independent cryptographic review, and deployment qualification remain unavailable or outside this packet and are not claimed.

TD-001 through TD-004 remain open. This checkpoint does not make ZK-ARCHE RFC-class, Common Contract conformant, rollback resistant, or deployment qualified.
