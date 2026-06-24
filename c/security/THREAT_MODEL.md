# ZK-ARCHE Threat Model

## Scope
This threat model covers the ZK-ARCHE setup and authentication protocols for IoT devices, gateways, and servers. It applies to both the Rust and C implementations.

## Assets
- Device long-term secret scalar.
- Device public identity and pseudonymous authentication identifier.
- Server long-term secret scalar and pinned server public key.
- Session keys derived after authentication.
- Role credential and role commitment/blinding value.
- Replay cache state and sequence/session identifiers.
- Pairing token or onboarding material, if used.

## Trust assumptions
- Ristretto255 discrete logarithm is infeasible for the adversary.
- SHA-256/HMAC/HKDF are secure for their intended uses.
- The random number generator on each platform provides cryptographically strong entropy.
- Long-term private keys are protected against direct extraction.
- The verifier uses the expected registered public key and role policy for the device.

## Adversary model
The network adversary may eavesdrop, replay, drop, delay, reorder, modify, and inject packets. The adversary may run fake clients and fake servers. The adversary may capture old transcripts and try to authenticate later. The adversary may attempt malformed packet attacks against parsers. The adversary may try downgrade, unknown-key-share, and man-in-the-middle attacks.

## Out of scope unless explicitly evaluated
- Physical key extraction from flash or RAM.
- Power/EM side-channel attacks.
- Fault injection against scalar arithmetic or RNG.
- Compromise of the server database.
- Malicious firmware installed before protocol execution.

## Security boundaries
- Protocol logic is portable.
- Transport, storage, and RNG are platform-specific trust boundaries.
- TOFU onboarding is a lower-assurance mode and must not be treated as equivalent to authenticated provisioning.
