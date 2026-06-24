# External Review Brief: Role-Membership Proof

## Review objective
Evaluate whether the role-membership proof is complete, sound, zero-knowledge, transcript-bound, replay-resistant, and safe under the selected role encoding.

## Materials to provide reviewers
- Protocol specification and wire format.
- Source files implementing role encoding, role commitment, rerandomization, and set-membership proof verification.
- Threat model and security goals.
- Test vectors for valid and invalid role proofs.

## Reviewer questions
1. Is the commitment construction binding and hiding under the stated assumptions?
2. Is the OR-proof challenge construction sound and domain separated?
3. Can a prover mix branches from different role sets or transcripts?
4. Does branch ordering leak role information?
5. Are invalid points, non-canonical scalars, or identity elements rejected consistently?
6. Does the proof remain zero-knowledge against a malicious verifier?
7. Is replay prevented by transcript, nonce, and session binding?
8. Does the implementation avoid accidentally revealing the real branch through timing or error behavior?

## Expected output
A short review memo that classifies findings as critical, high, medium, low, or informational, with recommended fixes.
