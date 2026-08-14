# ZK-ARCHE Implementation Requirements Skeleton

Status: draft skeleton.

Implementation requirements to make testable:

- bounded parsing and deterministic error handling;
- fixed-size buffers where practical in C hot paths;
- no unchecked integer overflow in length arithmetic;
- constant-time operations for secret-dependent cryptographic paths;
- RNG failure must fail closed;
- storage writes for credentials, registry, tickets, replay state, and enrollment grants must be atomic or explicitly recoverable;
- restart behavior must not silently weaken replay guarantees;
- side-channel and memory-safety evidence must be profile-specific;
- Rust and C implementations must preserve the same normative wire, transcript, proof, KDF, and key-confirmation semantics wherever both implement the feature.
