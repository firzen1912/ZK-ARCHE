# Security Goals and Assurance Claims

## Required goals
1. Device authentication: only a device knowing the registered private scalar can authenticate for the corresponding public key.
2. Server authentication: the client authenticates the server against a pinned or provisioned server public key.
3. Key secrecy: the established session keys are unknown to passive and active network attackers.
4. Key confirmation: both peers confirm they derived the same session keys over the same transcript.
5. Replay resistance: old AUTH_1/AUTH_2/AUTH_3 messages are rejected or handled idempotently as retransmissions.
6. Transcript binding: challenges, nonces, ephemeral keys, roles, and identities are bound into Fiat-Shamir/HKDF/HMAC contexts with domain separation.
7. Role authorization: the server accepts only if the role proof satisfies an allowed role set.
8. Role privacy: the role proof should reveal membership in an allowed set without unnecessarily revealing the exact role.

## Current assurance claim
The implementation should be described as a strong research prototype. It uses standard primitives and structured transcripts, but it is not yet formally verified, side-channel evaluated, externally audited, or production certified.

## Claims that must not yet be made
- Do not claim formal proof unless the ProVerif/Tamarin model and cryptographic proof are completed.
- Do not claim production readiness without fuzzing, replay testing, side-channel review, and external cryptographic review.
- Do not claim secure onboarding when TOFU is enabled.
