# ZK-ARCHE Test Vector Specification Skeleton

Status: draft skeleton.

Canonical source today:

```text
rust/test-vectors/0x0001/
```

Python mirrored fixture today:

```text
python/test-vectors/0x0001/
```

Vector categories to define:

- transcript bytes;
- PID derivation;
- Schnorr/auth proof;
- role proof and rerandomization;
- KDF and key confirmation;
- wire/header/TLV encodings;
- HELLO negotiation;
- SETUP/AUTH state-machine traces;
- negative vectors for mutation, replay, wrong sequence, wrong suite, wrong profile, malformed input, and downgrade attempts.

Vector changes require versioning and migration notes. Do not replace vector semantics silently.
