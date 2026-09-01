# Transport Binding and Adapter Authority Contract

Status: normative draft for `ZK-ARCHE-BIND` / zk225–zk230.

## Scope

This contract defines the boundary between transport adapters and protocol authority. It normalizes adapter- or channel-derived context without allowing routing metadata to become identity, authorization, trust, or enrollment authority.

`ZK-ARCHE-BIND` owns channel/exporter binding normalization. `ZK-ARCHE-CORE` owns canonical protocol identity/context encoding, `ZK-ARCHE-AUTH` owns authentication, `ZK-ARCHE-LINK` owns association/replay/key lifecycle/resumption, and `ZK-ARCHE-TRUST` owns authorization/trust/lineage/revocation decisions. A transport adapter only moves framed protocol bytes and reports explicitly characterized metadata.

## Mandatory invariants

1. A transport address (IP/port, URI, BLE address, serial endpoint, gateway handle, or equivalent) MUST NOT be treated as protocol identity.
2. Unauthenticated transport metadata MUST NOT be treated as authorization, trust, enrollment, lineage, revocation, or peer-identity authority.
3. Address changes MUST NOT by themselves change protocol identity or invalidate a cryptographically current binding.
4. If a selected profile requires a channel/exporter binding and no binding is available, the operation MUST fail closed.
5. If a binding is present, implementations MUST validate its integrity, freshness, AUTH-instance binding, peer-context binding, and profile-context binding before using it. A malformed, stale, or mismatched optional binding MUST NOT be silently ignored to manufacture an unbound success.
6. A valid binding is evidence about the association/context it covers. It is not sufficient authentication, authorization, trust mutation, or resumption authorization by itself.
7. TLS, DTLS, QUIC, OSCORE, EDHOC, or another secure transport MAY supply authenticated exporter/channel context in profiles that define it. No such transport is a mandatory dependency of the Common Contract.
8. Core P2P AUTH MUST remain possible for already-authorized peers without CA, DNS, Internet, cloud, manufacturer service, or gateway approval when the selected profile does not require an external channel binding.
9. Capability negotiation MUST NOT turn a required binding into an unbound mode or accept a weaker binding than the selected profile requires.

## Normalized decision

The binding classifier consumes these facts:

- `profile_requires_binding`
- `binding_present`
- `binding_integrity_valid`
- `binding_fresh`
- `auth_instance_match`
- `peer_context_match`
- `profile_context_match`
- `transport_address_as_identity`
- `transport_metadata_as_authority`

It returns exactly one disposition:

- `BOUND`: a supplied binding is current and bound to the selected AUTH instance, peer context, and profile context.
- `UNBOUND_ALLOWED`: no binding is present and the selected profile does not require one. Transport metadata remains advisory only.
- `REJECT`: the adapter/context attempts to become authority, a required binding is missing, or a supplied binding is invalid/stale/mismatched.

Reject precedence is deterministic: address-as-identity; transport-metadata-as-authority; required binding missing; binding integrity; binding freshness; AUTH-instance match; peer-context match; profile-context match.

## Adapter contract

Each adapter specification and qualification record MUST state: framing ownership; MTU/fragmentation behavior; ordering/duplicate behavior; retransmission ownership; address/context volatility; amplification exposure; which metadata is authenticated versus advisory; channel/exporter-binding availability; and target resource footprint where measured.

The existing UDP/TCP adapters remain byte transports. Socket addresses are routing handles, not peer identities. Future channel/exporter adapters must feed normalized facts into this BIND contract rather than creating a separate trust engine.

## Resumption interaction

A caller MAY map `BOUND` to the existing resumption classifier's `binding_valid=true` only when the resumption credential requires the same binding context. `UNBOUND_ALLOWED` may satisfy resumption only for a profile/credential that explicitly permits no channel binding. `REJECT` MUST NOT be converted into fresh authorization or persistent trust; callers may instead require a fresh full AUTH where lifecycle policy permits.

## Deterministic conformance corpus

`rust/test-vectors/state/transport-binding-v1.txt` is the canonical Rust/C decision corpus. It includes required/optional binding, invalid and stale bindings, context mismatches, forbidden address/metadata authority, and an address-change case whose current cryptographic binding remains accepted.

Passing this corpus establishes only decision parity for this normalized boundary. It does not establish secure exporter construction, transport security, end-to-end interoperability, external review, physical MCU evidence, or deployment qualification.
