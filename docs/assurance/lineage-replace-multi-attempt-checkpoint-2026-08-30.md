# Checkpoint: lineage replacement multi-attempt event qualification

Date: 2026-08-30
Roadmap owner: zk213
Claim class: reproducible Rust/C semantic evidence only

## Change under review

This checkpoint composes the existing attempt/confirmation-binding classifier with a shared wire-neutral multi-attempt event corpus. It targets stale-attempt confirmation, duplicate/reordered confirmation, asymmetric restart, simultaneous different attempts, and explicit clean retry without allocating production wire behavior.

Affected surfaces:

- `spec/lineage-replace-multi-attempt-events.md`;
- shared `MA-01` through `MA-14` canonical corpus;
- independent Rust and C event reducers/corpus consumers.

## Security review checklist

- Normal AUTH remains NO-LEARNING: unchanged.
- Authentication is not treated as authorization: local and peer replacement authorization remain explicit event state.
- Trust mutation is not performed by either event reducer.
- A stale confirmation is accepted only when its attempt is the side's current attempt; it cannot carry into a newer attempt.
- Changing the current attempt clears the prior confirmation binding on that side.
- Duplicate current-attempt observations/confirmations are idempotent.
- Restart clears volatile attempt/confirmation evidence for the affected side and therefore cannot manufacture convergence.
- Simultaneous different attempts remain fail closed through the existing `ATTEMPT_ID_MISMATCH` decision.
- Clean retry requires explicit abandonment of the conflicting volatile attempt state and fresh matching observations/confirmations.
- No transport address is used as protocol identity.
- No packet type, field encoding, suite, primitive, timeout, retransmission rule, or collision winner is allocated.

## Canonical negative coverage

`MA-02` shows old confirmation cannot complete `A2`. `MA-05` and `MA-11` keep restart state loss incomplete. `MA-06` requires reconfirmation after local restart. `MA-08` rejects simultaneous different attempts. `MA-10` rejects asymmetric stale/current attempt state. `MA-12` shows prior `A1` confirmation does not carry into `A2`. `MA-14` rejects unilateral advancement to a newer attempt. `MA-09` demonstrates that convergence after conflict requires an explicit clean retry followed by a matching current attempt and bilateral current confirmation.

## Validation boundary

The C reducer and canonical corpus were compiled and executed in isolation with GCC under `-std=c11 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Werror`, using the exact-current C attempt classifier semantics; all 14 cases passed. A clean repository checkout is unavailable in the current execution environment because direct `github.com` DNS resolution fails. Rust execution/rustfmt, full C/libsodium integration, sanitizers, cppcheck, formal synchronization/ProVerif, and release qualification are therefore unavailable and are not claimed.

TD-001 through TD-004 remain open. This checkpoint does not establish external cryptographic review, constrained-target measurements, formal completion, RFC-class documentation, malicious rollback resistance, Common Contract conformance, or deployment qualification.
