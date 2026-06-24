# Replay-Cache Test Plan

## Required negative tests
1. Reuse the same AUTH_1 packet with identical `pid || nonce_c || eph_c`; the verifier must reject or treat it as an idempotent retransmission only if the session state explicitly allows it.
2. Replay AUTH_1 after a completed session; the verifier must reject it.
3. Replay AUTH_2 to a different client session; the client must reject it because transcript/HMAC verification fails.
4. Replay AUTH_3 after AUTH_ACK; the server must reject or idempotently return cached ACK only for the same session/sequence.
5. Change `seq` while keeping old payload; transcript or state verification must reject.
6. Reorder AUTH_2 before AUTH_1; the receiver must reject due to missing state.
7. Reuse `nonce_c` with a new `eph_c`; policy should reject or log as suspicious.
8. Reuse `eph_c` with a new `nonce_c`; policy should reject or log as suspicious.

## Required positive tests
1. A valid first authentication succeeds.
2. A retransmission marked with the retransmit flag for the same `(session_id, seq)` receives the same cached response if retransmission support is implemented.
3. Fresh authentication with fresh nonce and ephemeral key succeeds.

## Evidence to collect
- Test name.
- Expected result.
- Actual result.
- Packet fields replayed or modified.
- Whether receiver returned an error, ignored packet, or cached response.
