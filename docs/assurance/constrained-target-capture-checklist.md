# Constrained-target physical capture procedure

Status: repository-owned preparation for TD-002. This procedure does **not** constitute physical-target evidence and does not promote any target/profile to `MEASURED`, `COMMON-CONFORMANT`, or `DEPLOYMENT-QUALIFIED`.

The authoritative evidence container is `constrained-target-benchmark-manifest.schema.json`. A completed run must bind observations to the exact repository commit and preserve raw artifacts sufficient to reproduce each reported value.

## 1. Before the run

Record, without inference:

- exact 40-character repository commit, branch, and dirty/clean state;
- board model and revision, MCU/SoC, CPU, configured clock, physical RAM and flash;
- RTOS/runtime where applicable;
- compiler/toolchain and exact version, build profile, flags, and Rust/C implementation lane;
- cryptographic library/version, acceleration path, and whether software fallback is enabled;
- entropy source, health-test posture, DRBG/reseed behavior, key-generation mode, stored representation, storage location, self-test posture, and zeroization assumptions;
- secure-boot and debug state plus rollback, cloning, and reprovisioning assumptions;
- transport, MTU, reliability semantics, and the exact transport context used by the run;
- dependency versions that can affect wire, crypto, storage, timing, or memory behavior.

Unknown values must remain explicitly unknown in run notes until resolved. Do not substitute host defaults, development-board marketing specifications, simulator observations, or values from a different firmware build.

## 2. Preserve the build identity

Retain enough information to reconstruct the tested binary. At minimum preserve the exact source commit, toolchain identity, build command/profile/flags, dependency versions, and a digest of the produced firmware/binary in the raw-artifact set.

A measurement from a binary that cannot be related back to the manifest's source/build identity is not sufficient TD-002 clearing evidence.

## 3. Wire capture

Capture the actual protocol exchange used for the measurement and retain the raw capture or deterministic device log. Report wire bytes from the observed exchange, not from struct sizes or source-code estimates.

Record whether framing, transport headers, retransmissions, fragmentation, and lower-layer overhead are included. Comparisons must use the same accounting boundary.

## 4. Memory and flash capture

Measure and retain the method/output for:

- peak stack;
- peak heap;
- static RAM;
- firmware/code flash attributable to the tested build.

If the platform cannot measure a category reliably, do not replace it with an estimate and do not report the target as fully measured. Preserve the limitation in the raw artifacts and run notes.

## 5. Latency and cycle capture

Pin the clock/configuration and measurement boundary before collecting timing. Retain raw samples, not only an average. The manifest's `sample_count` must equal the retained population used to derive the reported value.

Record interrupt/scheduler conditions, crypto acceleration state, transport path, warm/cold-cache or equivalent execution assumptions, and any preprocessing excluded from the timing boundary.

A host benchmark, simulator result, or a different MCU cannot be relabeled as the target's latency or cycle evidence.

## 6. Entropy and key-storage observations

Exercise the exact entropy/DRBG/key-storage path used by the tested profile. Record configuration and observable health/error behavior. Do not infer RNG quality, secure erasure, anti-cloning, or rollback resistance solely because a vendor API exists.

Any claim about secure storage, boot state, debug lock, rollback protection, or zeroization must be supported by the physical configuration and retained observation appropriate to that claim.

## 7. Lifecycle qualification

For the tested target/profile, explicitly record whether each lane was actually exercised:

- restart and replay-state continuity;
- rollback detection/recovery;
- revocation handling;
- authorization/lineage/epoch invalidation where the selected profile uses them.

A `false` value is valid evidence that the behavior was not tested; it must not be rewritten as success. Text describing replay state after restart must reflect observation from the physical run.

## 8. Raw-artifact minimum

Each measured manifest should reference retained artifacts sufficient to audit the reported values. Prefer immutable or content-addressed artifacts. The set should normally include:

```text
build command/configuration + firmware digest
compiler/linker size output
wire capture or deterministic protocol log
stack/heap instrumentation output
raw latency/cycle samples
restart/replay observation
rollback observation when tested
revocation observation when tested
platform security/configuration evidence relevant to claims
```

Artifact names alone do not prove that the files came from the stated target. Preserve capture provenance with them.

## 9. Evidence-status transition

Use `measured-unreviewed` only after the physical run has occurred and the manifest values are backed by retained raw artifacts. Use `reviewed` only after a reviewer has checked the manifest against those artifacts and the exact build/target context.

The schema currently also contains the value `planned`. Treat a `planned` manifest strictly as scheduling/configuration metadata: it is **not** a measurement record and must not be used to populate roadmap scores or maturity claims. Never invent nonzero samples or placeholder raw artifacts merely to make a planned record look measured. If the schema cannot represent an unmeasured plan without synthetic measurement fields, keep the plan outside the evidence manifest until a real run exists.

## 10. Promotion boundary

A physical capture can support only the target, build, profile, crypto path, transport context, and lifecycle conditions actually exercised. It does not establish:

- independent cryptographic review (TD-001);
- formal-analysis completeness (TD-003);
- RFC-class documentation (TD-004);
- interoperability with another implementation unless that interoperability was executed;
- constant-time behavior or computational proof soundness;
- field/product readiness beyond the measured configuration.

TD-002 should advance only from reproducible physical evidence, never from this procedure, schema validity, planned manifests, simulation, host tests, or documentation completeness.