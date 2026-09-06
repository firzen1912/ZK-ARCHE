# TLS Exporter Channel-Binding Contract

Status: **draft normative work** for ZK-ARCHE-BIND and roadmap `zk228`. This document defines a candidate TLS/mTLS exporter binding. It does not make TLS mandatory, does not make TLS identity authoritative for ZK-ARCHE, does not promote any draft profile, and does not claim current Rust/C wire interoperability.

## 1. Scope and authority boundary

TLS is an optional transport/channel binding beneath the ZK-ARCHE Common Contract. Native P2P AUTH between already-authorized peers MUST remain possible without TLS, PKI, DNS, CA, cloud, gateway, or Internet dependencies when the selected profile permits the underlying transport.

A successful TLS handshake, certificate validation, transport address, SNI value, ALPN value, proxy identity, or exporter value MUST NOT by itself create ZK-ARCHE trust or authorization. ZK-ARCHE AUTH/TRUST remains the protocol authority. The exporter binding only proves that a particular ZK-ARCHE AUTH instance is cryptographically bound to a particular TLS security context.

## 2. Exporter construction

A TLS-capable binding profile using this contract MUST derive exactly 32 octets using the TLS exporter interface with:

```text
label   = "EXPORTER-ZK-ARCHE-v1"
context = ZKARCHE-EXPORTER-CONTEXT-v1
length  = 32
```

The context MUST be non-empty and canonically encoded. Implementations MUST NOT substitute a TLS connection identifier, certificate hash, socket tuple, or application-provided opaque string for the canonical context.

## 3. Canonical exporter context

`ZKARCHE-EXPORTER-CONTEXT-v1` is the SHA-256 digest of the following length-delimited structure:

```text
ASCII("ZKARCHE-EXPORTER-CONTEXT-v1")
|| u16be(len(application_id)) || application_id
|| u16be(len(alpn))           || alpn
|| u16be(len(deployment_id))  || deployment_id
|| u16be(len(initiator_id))   || initiator_id
|| u16be(len(responder_id))   || responder_id
|| u16be(protocol_version)
|| u16be(suite_id)
|| u16be(profile_id)
|| u16be(len(auth_instance_id)) || auth_instance_id
|| u16be(len(auth_transcript_hash)) || auth_transcript_hash
```

All byte strings are exact protocol bytes, not locale-dependent text transformations. Empty `application_id`, `deployment_id`, endpoint identity/commitment, `auth_instance_id`, or `auth_transcript_hash` is invalid for this binding. `alpn` MAY be empty only when the selected transport profile explicitly permits TLS without ALPN; the empty value remains length-bound in the context.

`initiator_id` and `responder_id` are ZK-ARCHE protocol identities or cryptographic commitments in handshake role order. They are not IP addresses, DNS names, certificate subject names, BLE addresses, or other transport locators unless a separate ZK-ARCHE specification cryptographically maps that value into protocol identity.

`auth_instance_id` MUST be fresh or otherwise unique within the lifetime/reuse bounds of the selected profile. Repeating the same AUTH transcript on the same resumed TLS connection MUST NOT cause two logically distinct AUTH instances to share the same exporter context.

`auth_transcript_hash` MUST cover the ZK-ARCHE transcript state required by the selected version/profile before channel binding is accepted. A profile MUST specify the exact transcript checkpoint; implementations MUST NOT choose different checkpoints opportunistically.

## 4. Acceptance rule

A peer MUST accept a TLS exporter channel binding only when all of the following hold:

1. the selected ZK-ARCHE version/profile permits this binding method;
2. the TLS exporter operation succeeds for the active TLS security context;
3. the locally reconstructed canonical context is valid;
4. application/ALPN, deployment/domain, endpoint identities/commitments, suite, profile, AUTH-instance identity, and transcript hash equal the values bound into the ZK-ARCHE AUTH instance;
5. both peers confirm the same 32-octet exporter-derived binding through the ZK-ARCHE transcript/context mechanism;
6. no lifecycle rule has already invalidated the AUTH/association authority.

Failure of any mandatory condition MUST fail closed for a profile requiring TLS exporter binding. A profile that treats TLS binding as optional MAY negotiate a different registered binding method, but MUST NOT silently reinterpret a failed exporter binding as `NO-CHANNEL-BINDING`.

## 5. Multiple AUTH instances and TLS resumption

The exporter binding is per **ZK-ARCHE AUTH instance**, not merely per TLS connection.

Two AUTH instances carried by one TLS connection MUST have distinct `auth_instance_id` values and therefore distinct contexts. A TLS 1.3 resumed connection is a new TLS security context for exporter purposes; ZK-ARCHE MUST reconstruct and verify the full canonical context for the AUTH instance carried over it.

A ZK-ARCHE resumption fast path MUST NOT reuse a prior exporter value as authority for a new TLS connection or a new AUTH/resumption instance. Any permitted ZK-ARCHE resumption binding must incorporate the current TLS exporter context and satisfy the independent authorization-aware resumption lifecycle contract.

## 6. Termination, proxy, and transport changes

When TLS terminates at a proxy, gateway, load balancer, service mesh, or other intermediary, the exporter is bound to that TLS hop. An intermediary MUST NOT assert an exporter value for an end-to-end TLS context it does not possess. A deployment that needs end-to-end ZK-ARCHE identity across TLS termination MUST rely on ZK-ARCHE's own end-to-end AUTH/transcript semantics and explicitly specify what hop-local binding, if any, is retained.

Changing an IP address, port, interface, or other transport locator does not change ZK-ARCHE identity by itself. Conversely, retaining a transport locator does not preserve authority after ZK-ARCHE lifecycle invalidation.

## 7. Required deterministic qualification

Before this binding can be promoted from draft, shared Rust/C conformance evidence MUST include at least:

- positive same-connection/same-instance derivation;
- second AUTH instance on the same TLS connection produces a distinct binding;
- wrong `application_id` / ALPN rejects;
- wrong deployment/domain rejects;
- initiator/responder swap rejects;
- wrong endpoint identity/commitment rejects;
- wrong suite or profile rejects;
- stale/reused `auth_instance_id` outside allowed bounds rejects;
- wrong transcript hash/checkpoint rejects;
- TLS-resumption fixture reconstructs a new current binding rather than reusing the prior exporter;
- cross-protocol/cross-application context substitution rejects;
- proxy/termination fixture demonstrates that hop-local exporter identity is not promoted to ZK-ARCHE protocol authority;
- required-binding negotiation cannot downgrade to no binding after exporter failure.

Fixtures MAY use deterministic synthetic exporter inputs to test context construction, but such fixtures are not evidence of a live TLS stack or transport interoperability. Promotion to `INTEROPERABLE` requires actual Rust/C adapter execution against a supported TLS implementation.

## 8. Security and privacy considerations

The explicit application, deployment, endpoint, suite/profile, instance, and transcript fields prevent one exporter context from being silently reused across protocol instances with different security meaning. Implementations SHOULD minimize stable plaintext identifiers in application-visible diagnostics; the exporter context itself is hashed before use as TLS exporter context.

This construction does not prove certificate policy correctness, endpoint authorization, privacy of the underlying TLS metadata, constant-time behavior, RNG quality, or ZK-ARCHE authorization correctness. Those remain separate evidence obligations.

## 9. Promotion gate

This contract remains draft until all of the following exist:

```text
canonical Rust/C context encoder
+ deterministic positive/negative shared corpus
+ supported TLS adapter integration
+ cross-instance and TLS-resumption execution
+ downgrade/termination negative evidence
+ registry allocation for the binding method
+ exact-head qualification evidence
```

The presence of this document alone does not raise `zk228` to complete and does not make TLS a Common Contract dependency.
