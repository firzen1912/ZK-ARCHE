# ZK-ARCHE Implementation Requirements Skeleton

Status: draft skeleton.

Implementation requirements to make testable:

- bounded parsing and deterministic error handling;
- fixed-size buffers where practical in C hot paths;
- no unchecked integer overflow in length arithmetic;
- constant-time operations for secret-dependent crypto paths;
- RNG failure must fail closed;
- storage writes for credentials, registry, tickets, replay state, and enrollment grants must be atomic or explicitly recoverable;
- restart behavior must not silently weaken replay guarantees;
- side-channel and memory-safety evidence must be profile-specific;
- Python is a reference/interoperability lane and not a constrained-device target.
