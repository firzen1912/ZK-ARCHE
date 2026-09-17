# Association Admission Formal Traceability

This document binds the current secure-association admission and retained-authority surfaces to TD-003 without claiming a fresh symbolic result. It is traceability evidence: model presence and implementation/test mappings do not by themselves make the lifecycle `FORMALLY ANALYZED`.

## Current synchronized model surface

The repository carries byte-identical Rust/C copies of:

- `models/proverif/zk_arche_association_admission_draft.pv` — admission composition and NO-LEARNING rejection boundary;
- `models/proverif/zk_arche_retained_authority_draft.pv` — temporal retained-use authority after association establishment;
- adjacent AUTH-v3, replay-continuity, lineage-replacement, and DATA lifecycle models for owning subsystem properties.

The association-admission model requires establishment to imply completed AUTH, a pre-existing trust record, present/fresh/current-generation authorization, current revocation state, non-revoked holder state, current lineage, replay/restart/usage-counter continuity, valid binding, and rollback-clear state. It excludes establishment when normal AUTH requests trust mutation or explicit revocation is observed.

The retained-authority model separately requires protected use to remain under current lifecycle authority and excludes retained use after authorization-generation drift, stale revocation state, explicit revocation, lineage drift, replay/restart/usage-counter continuity loss, or required-binding loss.

These models consume authoritative lifecycle facts. They do not derive revocation convergence, persistent monotonicity, channel cryptography, parser correctness, proof soundness, storage physics, or key erasure.

## Runtime and qualification surfaces

Canonical implementation/evidence surfaces include:

- `rust/crates/proto/src/association_admission.rs`;
- `c/src/proto/association_admission.c`;
- `rust/test-vectors/state/association-admission-v4.txt` and independent Rust/C consumers;
- `rust/crates/proto/tests/association_admission_temporal.rs` for temporal invalidation after an initially admissible association.

The temporal Rust qualification closes an earlier traceability gap by making post-admission authority drift executable: authorization-generation change, stale/explicit revocation, lineage change, replay/restart/usage-counter continuity loss, binding invalidation, NO-LEARNING trust-mutation rejection, and rollback-dominant failure are represented as state changes rather than only independent static classifier rows.

This does **not** establish C temporal-test parity or a fresh exact-head PASS. The underlying classifier and canonical static corpus remain cross-language; the temporal Rust test is additional qualification evidence whose C counterpart remains dependency-ready work.

## Property mapping

| Property | Model | Runtime / evidence | Remaining assurance gap |
|---|---|---|---|
| establishment requires completed AUTH | association-admission | Rust/C classifier + v4 corpus | composition with cryptographic AUTH remains abstracted |
| normal AUTH is NO-LEARNING | association-admission | Rust/C classifier + corpus + Rust temporal test | trust-store persistence/mutation implementation is outside model |
| authorization generation must remain current | admission + retained-authority | Rust/C classifier + corpus + Rust temporal test | provenance namespace and durable generation storage remain abstracted |
| stale/explicit revocation invalidates authority | admission + retained-authority | Rust/C classifier + corpus + Rust temporal test | convergence timing/distribution is not proved |
| lineage drift invalidates authority | admission + retained-authority | Rust/C classifier + corpus + Rust temporal test | durable lineage state and delegation semantics remain separately owned |
| replay continuity loss invalidates authority | admission + retained-authority + replay-continuity | Rust/C classifier + corpus + Rust temporal test | persistent replay-store equivalence remains open |
| restart continuity loss invalidates authority | admission + retained-authority | Rust/C classifier + corpus + Rust temporal test | crash consistency/recovery physics remain open |
| usage-counter continuity loss invalidates authority | admission + retained-authority | Rust/C classifier + corpus + Rust temporal test | monotonic storage/key-erasure evidence remains open |
| required binding loss invalidates authority | admission + retained-authority | Rust/C classifier + corpus + Rust temporal test | live channel/exporter composition remains outside model |
| rollback suspicion fails closed | association-admission boundary + runtime precedence | Rust/C classifier/corpus + Rust temporal precedence test | rollback attacker and physical anti-rollback mechanism remain incomplete |

Deterministic failure-reason precedence is a classifier/corpus property, not a ProVerif theorem. Symbolic property coverage must not be described as proving implementation-specific reason ordering.

## Exact-result gate

A `FORMALLY ANALYZED` claim for an edited/new model requires retained evidence bound to the exact model blob and repository head, including tool/version, query identifiers, attacker assumptions, result/counterexample output, owning specification, Rust/C implementation mapping, test/vector mapping, and explicit abstraction gaps.

Cloud-runner inability to execute ProVerif is `UNAVAILABLE`, not `RED`, and does not invalidate the user-confirmed green local validation baseline. It also cannot be converted into a fresh formal PASS.

## Explicit abstraction gaps

The synchronized models do **not** establish:

- computational soundness or independent review of the role-membership proof;
- parser/wire equivalence or memory/constant-time safety;
- RNG/entropy quality;
- durable monotonic storage, atomicity, or physical rollback resistance;
- revocation-distribution convergence bounds;
- live TLS/DTLS/channel exporter correctness;
- key erasure/transport teardown after authority invalidation;
- MCU resource/physical-target behavior;
- privacy/unlinkability outside modeled events;
- deployment qualification or RFC/IETF status.

## Current claim

Association admission is **IMPLEMENTED and cross-language vector-governed** for the current classifier surface. Temporal invalidation now has an executable Rust qualification surface, and synchronized symbolic admission/retained-authority models cover the corresponding abstract lifecycle properties. The complete association lifecycle remains **not freshly FORMALLY ANALYZED at exact-current HEAD** until the current synchronized models are executed with retained exact-model results; C temporal parity, persistence/recovery equivalence, revocation convergence, live binding composition, and the other abstraction gaps above also remain open.
