# P2P bounded delegation checkpoint — 2026-09-01

This checkpoint records a narrow `zk239` advancement: explicit bounded delegation now has matching Rust/C decision semantics and a canonical negative corpus. It does not promote `p2p-iot-core` or claim physical cross-class interoperability.

Implemented facts include pre-existing issuer trust, authenticated holder, explicit verified-grant presence, scope/audience/deployment binding, validity and epoch freshness, revocation freshness, explicit revocation, lineage freshness, delegation depth, explicit redelegation permission, and rollback suspicion.

The corpus contains 17 cases. Two positive cases cover direct bounded acceptance and explicitly permitted redelegation within depth. Negative cases cover every individual guard, including the non-transitive default, stale revocation/epoch/lineage, depth overflow, forbidden redelegation, and rollback suspicion.

Local validation executed the C classifier against the exact drafted corpus under `gcc -std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror` and returned `p2p bounded delegation corpus: ok`. Rust execution was unavailable in this environment, so Rust compile/test evidence is not claimed.

Evidence state: IMPLEMENTED advanced for the decision layer; TESTED narrowly in C; INTEROPERABLE and COMMON-CONFORMANT remain incomplete. No physical MCU measurement, external review, formal proof, deployment evidence, or RFC status is implied.
