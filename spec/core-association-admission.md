# CORE secure-association admission postcondition

Status: implementation-backed draft for the current `dev` baseline. This document defines a wire-neutral CORE/LINK postcondition; it does not create a new trust store, authorization engine, revocation engine, replay store, lifecycle authority, or transport identity.

## 1. Purpose

A completed cryptographic AUTH exchange is necessary but not sufficient to establish or retain a secure association. Association admission MUST compose the current decisions of the owning AUTH, TRUST, LINK, lifecycle, and BIND layers and MUST fail closed when any mandatory prerequisite is unsafe.

The classifier is a postcondition over authoritative facts. It MUST NOT independently learn peers, create trust, repair missing state, infer authorization from authentication, synthesize authorization-generation provenance, or treat transport metadata as identity.

## 2. Authoritative inputs

| Fact | Owning authority |
|---|---|
| `auth_complete` | AUTH completion/key-confirmation state |
| `preexisting_trust_record` | locally authoritative TRUST state |
| `authorization_present` | TRUST authorization state |
| `authorization_fresh` | authorization freshness/policy state |
| `authorization_generation_bound` | authenticated authorization evidence/local metadata |
| `authorization_generation_current` | current local authorization-generation lifecycle authority |
| `revocation_current` / `explicitly_revoked` | current local revocation view |
| `lineage_current` | TRUST lineage/replacement state |
| `replay_continuity_current` | LINK replay continuity state |
| `restart_continuity_current` | persistent LINK/lifecycle restart continuity state |
| `usage_counter_continuity_current` | LINK key-usage/counter continuity state |
| `binding_required` / `binding_valid` | selected profile plus BIND decision |
| `rollback_suspected` | persistent lifecycle-state integrity/recovery state |
| `trust_mutation_requested` | caller intent at the normal AUTH boundary |

Callers MUST derive these facts from the corresponding owning subsystem and MUST NOT manufacture a successful fact merely to retain an association.

`authorization_fresh`, `authorization_generation_bound`, and `authorization_generation_current` are distinct. Freshness expresses policy/time validity. Generation binding proves, directly or through authenticated local metadata, which local authorization generation the evidence belongs to. Generation currentness compares that authenticated binding with the current generation supplied by the single lifecycle authority defined by `authorization-generation-lifecycle.md`.

A successful AUTH exchange, cached association, transport address, socket, channel metadata, peer assertion, optional infrastructure, or cryptographically valid but generation-unbound credential MUST NOT synthesize `authorization_generation_bound=true`. An unbound generation and a bound-but-stale generation are independent fail-closed conditions.

`replay_continuity_current`, `restart_continuity_current`, and `usage_counter_continuity_current` are distinct. Replay state may be internally well-formed while persistent restart/recovery continuity is unknown or stale. An association MUST NOT survive that uncertainty merely because its cryptographic AUTH previously completed.

Key-usage/counter continuity is a separate LINK fact again. Replay and restart continuity may both be current while the key-usage counter for the association cannot be shown to have advanced monotonically — for example after a counter reset, an unmirrored counter write, or a recovered persistence image whose counter position is unknown. Losing that continuity is a nonce/key-reuse hazard rather than a replay-window question, so it MUST fail closed independently. This matches `resumption-authorization-decision.md` and `transport-continuation-decision.md`, which already reject on the same fact; CORE admission MUST NOT be the one surface that admits an association whose key-usage continuity is unknown.

## 3. Decision

The only successful result is `ESTABLISH`. Every other result is `FAIL_CLOSED` with a deterministic reason.

Association admission MUST fail closed, in precedence order, when:

1. rollback is suspected;
2. normal AUTH is being asked to mutate trust;
3. AUTH is incomplete;
4. no pre-existing local trust record exists;
5. scoped authorization is absent;
6. authorization state is stale;
7. authorization-generation provenance is absent or unauthenticated;
8. the authentically bound authorization generation is not current;
9. the revocation view is stale;
10. the holder is explicitly revoked;
11. authorization/trust lineage is stale;
12. replay continuity is not current;
13. restart/recovery continuity is not current;
14. key-usage/counter continuity is not current; or
15. the selected profile requires a binding and the binding is not valid.

Only when every mandatory condition is satisfied may the association be established or retained.

## 4. NO-LEARNING AUTH

Normal AUTH is NO-LEARNING. A mathematically valid proof from an unknown peer MUST NOT create a trust record, enrollment grant, delegation, authorization record, or authorization-generation binding. If association admission observes `trust_mutation_requested=true`, it MUST fail closed.

Trust mutation belongs to explicit ENROLL, commissioner/grant, reviewed rekey/re-registration, revocation, or equivalent lifecycle transitions. A successful association-admission result confirms only that locally authoritative prerequisites were already satisfactory.

## 5. Re-evaluation, authority loss, and invalidation

The same postcondition applies to retaining an existing association. When locally authoritative lifecycle state changes, implementations MUST re-evaluate the association before security-sensitive continued use when the profile requires that state to be current.

An existing association MUST NOT remain security-authoritative merely because traffic keys still exist when authorization becomes stale, generation provenance becomes unavailable, the local generation advances, the holder becomes revoked, lineage becomes stale, replay, restart, or key-usage continuity is lost, rollback is suspected, or a required channel binding becomes invalid.

A `FAIL_CLOSED` result during retention re-evaluation removes the association's authority for new protected application traffic immediately. Callers MUST NOT continue transmitting or accepting security-sensitive application data under that association merely because cryptographic key material or a transport connection remains present.

Existing key material MAY remain transiently resident only for a narrowly scoped shutdown, authenticated close, or implementation-owned cleanup path when the selected profile explicitly permits that behavior. Such transient retention MUST NOT authorize new application data, repair stale lifecycle facts, advance replay/key-usage state as if the association were current, or convert a failed association back to `ESTABLISH` without a fresh successful admission evaluation.

Successful reauthentication does not by itself restore authorization-generation provenance/currentness, restart continuity, key-usage continuity, revocation freshness, lineage freshness, or required channel binding. Those facts remain owned by their corresponding lifecycle authorities and MUST be established independently before association authority is restored.

This postcondition deliberately distinguishes **authority invalidation** from **key erasure mechanics**. The classifier defines when an association ceases to be security-authoritative; concrete key-zeroization timing, transport shutdown ordering, authenticated-close behavior, and any wire alert remain LINK/state-machine responsibilities and require their own implementation and negative tests. Implementations MUST NOT use the absence of an immediate erasure primitive as justification for continuing protected application traffic after `FAIL_CLOSED`.

## 6. Binding and transport independence

When `binding_required=false`, optional binding metadata MUST NOT become implicit authorization or identity authority. When `binding_required=true`, an invalid or missing effective binding MUST fail closed.

Transport addresses, sockets, MAC addresses, connection identifiers, radio endpoints, or gateway metadata do not satisfy `preexisting_trust_record` or `authorization_generation_bound` and cannot override an invalid BIND decision.

## 7. Infrastructure independence

Association admission is based on local security evidence. Availability of a CA, cloud service, central registry, DNS, gateway, Internet connection, or manufacturer service is not an input to this classifier.

Infrastructure may synchronize policy or revocation state. If local state exceeds the permitted freshness bound, the resulting stale fact causes fail-closed behavior; infrastructure availability itself does not become protocol authority and cannot synthesize generation provenance.

## 8. Canonical decision corpus

`rust/test-vectors/state/association-admission-v4.txt` is the canonical Rust/C decision corpus for this postcondition. Versions 1 through 3 are retained as historical evidence.

Version 4 covers successful admission plus negative cases for incomplete AUTH, missing trust, missing/stale authorization, unbound authorization generation, stale bound generation, stale revocation, explicit revocation, stale lineage, replay-continuity loss, restart-continuity loss, key-usage-continuity loss, required-binding failure, rollback suspicion, and attempted trust mutation.

`rust/models/proverif/zk_arche_association_admission_draft.pv` mirrors each mandatory fact as a correspondence query, so every fact above is required for `AssociationEstablished` in the symbolic model as well as in both implementations.

The current corpus proves the admission decision, not caller-side zeroization or transport teardown. The new authority-loss requirement in Section 5 therefore strengthens normative lifecycle semantics without claiming new executable evidence for key erasure, authenticated close, or shutdown ordering.

The corpus is decision-level evidence only. It is not byte-level handshake interoperability, physical constrained-target evidence, formal proof, or independent cryptographic review.
