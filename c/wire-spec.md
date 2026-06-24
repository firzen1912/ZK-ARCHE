# IoT-Auth ZK-ARCHE v2

**Wire-Format & Interoperability Specification**

_Version 0.2.0 — April 2026_

_Transport-agnostic authentication with pseudonymous session binding_

## Abstract

This document specifies the wire format, message flows, capability negotiation, cryptographic bindings, error codes, and conformance requirements for the IoT-Auth ZK-ARCHE v2 protocol. The goal of this revision is heterogeneous interoperability: any two implementations that conform to this specification must be able to interoperate regardless of programming language, hardware platform, or transport binding (UDP, TCP, CoAP, BLE-L2CAP, serial gateway, or future transports).

The protocol provides mutual authentication between a resource-constrained device and a server, with pseudonymous per-session identifiers, zero-knowledge role-set proofs, and authenticated key confirmation. Cryptographic primitives are defined over ristretto255 and are identified by a stable cipher-suite registry, allowing future agility without breaking existing deployments.

## Contents

- 1. Introduction and scope
- 2. Terminology
- 3. Protocol overview
- 4. Wire format
- 5. Capability negotiation
- 6. Transcript canonicalization
- 7. Cipher-suite registry and algorithm agility
- 8. Error code registry
- 9. Timing and retry policy; profiles
- 10. Test vectors and conformance
- 11. Conformance checklist
- Appendix A. Domain separators
- Appendix B. Change log

## 1. Introduction and scope

### 1.1 Goals

The ZK-ARCHE v2 protocol targets IoT deployments where devices are constrained (flash, RAM, power), networks are unreliable (UDP, lossy links, intermittent connectivity), and observability tolerates little operator intervention. This specification has four concrete goals:

1. Transport independence. The wire format travels unchanged across UDP datagrams, TCP length-prefix streams, CoAP requests, BLE attribute writes, or any future binding. A single interoperability test matrix covers all transports.
1. Cryptographic agility. Every message is stamped with a cipher-suite identifier from a stable registry. Deployments may adopt new curves, hashes, or post-quantum KEMs by registering new suite IDs without renumbering existing messages.
1. Deterministic canonicalization. Transcripts, point encodings, scalar encodings, and session pseudonyms have byte-for-byte stable definitions. A reference test-vector corpus permits cross-language validation with zero ambiguity.
1. Clear failure surfaces. Every protocol error has a stable 16-bit code. Peers that implement the protocol in different languages must report the same error on the same inputs, which is essential for fleet-scale diagnostics.

### 1.2 Non-goals

- Confidentiality of message contents: the protocol authenticates, but transport-layer confidentiality (if required) is expected to be provided by DTLS, QUIC, or an equivalent tunnel.
- Forward secrecy guarantees beyond the ephemeral ECDHE exchange documented in §3.3.
- Device attestation of hardware identity (TPM, secure-element remote attestation). These are complementary and MAY be carried in a future TLV extension.

### 1.3 Document conventions

Key words MUST, MUST NOT, REQUIRED, SHALL, SHALL NOT, SHOULD, SHOULD NOT, RECOMMENDED, MAY, and OPTIONAL are to be interpreted as described in RFC 2119 / RFC 8174.

All multi-byte integers on the wire are little-endian. All elliptic-curve points are 32-byte compressed ristretto255. All scalars are 32-byte canonical ristretto255 scalar encodings.

## 2. Terminology

| Term | Definition |
| --- | --- |
| device | The constrained party initiating authentication. Holds a long-term secret and a pinned server public key. |
| server | The verifying party. Holds a long-term static secret and a registry of enrolled devices. |
| device_id | 32-byte public identifier derived from the device root. Used only during enrollment, never during online authentication. |
| pid | 32-byte per-session pseudonym H(device_pub \|\| nonce_c \|\| eph_c \|\| server_pub). Binds every transcript for the session. See §6.3. |
| setup | Enrollment flow. Issues long-term credentials to the device and pins the server raw public key. Three round-trips. |
| auth | Online authentication flow. Produces a confirmed session key. Three round-trips. |
| suite | Cipher-suite identifier (u16) drawn from the registry in §7. |
| profile | Named bundle of timing, resource, and feature limits (§9.3). |
| role_commitment | Pedersen-style commitment C = g^role · h^blind carried in the registry, used in CDS role-set proofs. |
| session_id | 16-byte peer-scoped transport correlation identifier. Carried in every packet header. |

## 3. Protocol overview

### 3.1 Layering

This specification is layered so heterogeneous implementations can map it onto their environment without interpreting parts that do not concern them:

| Layer | Responsibility | Neutral ? |
| --- | --- | --- |
| A. Semantics | Setup and auth state machines; Schnorr, rerand, CDS-OR; KDF; KC. | Yes |
| B. Framing | 24-byte header, packet types, TLV codec, payload encoders. | Yes |
| C. Transport | UDP datagrams, TCP length-prefix streams, CoAP, BLE, serial. | Binding |
| D. Storage | Credential, registry, replay-cache backends. | Binding |

Layers A and B are fully specified here. Layers C and D are defined via abstract traits; a reference UDP binding and a reference filesystem store are provided, but conforming implementations may substitute any binding that preserves the per-message byte boundary and the byte-for-byte storage layout described in §4 and §5.

### 3.2 Flows

The protocol defines two flows. Setup is run once, at enrollment time. Auth is run for every authenticated session.

#### 3.2.1 Setup

```text
device                                                    server
  |                                                          |
  |---- SETUP_1 ---------------------------------------------->|
  |     (pairing_token?, device_id, device_pub,               |
  |      client_nonce, role_commitment)                       |
  |                                                           |
  |<--- SETUP_2 ----------------------------------------------|
  |     (server_nonce, setup_challenge, server_pub, proof_s)  |
  |                                                           |
  |---- SETUP_3 --------------------------------------------->|
  |     (proof_c)                                             |
  |                                                           |
  |<--- SETUP_ACK --------------------------------------------|
  |     (0x01)                                                |
```

#### 3.2.2 Auth

```text
device                                                    server
  |---- AUTH_1 ---------------------------------------------->|
  |     (pid, proof_c, nonce_c, eph_c,                        |
  |      c_prime, rerand_proof, role_set_branches*)           |
  |                                                           |
  |<--- AUTH_2 ----------------------------------------------|
  |     (server_pub, proof_s, nonce_s, eph_s, tag_s)          |
  |                                                           |
  |---- AUTH_3 --------------------------------------------->|
  |     (tag_c)                                               |
  |                                                           |
  |<--- AUTH_ACK --------------------------------------------|
  |     (0x01)                                                |
```

### 3.3 Cryptographic summary

- Identity proof. Schnorr over ristretto255. Device proves knowledge of x such that device_pub = g·x, bound in auth to pid rather than device_id.
- Server proof. Schnorr over ristretto255. Server proves knowledge of its static secret bound to nonce_s and eph_s.
- Role privacy. The stored role commitment C = g·role + h·blind is rerandomized per session to C′ = C + h·delta. The device proves (a) C′ is a rerandomization of a registered C and (b) C′ commits to some role in the allowed-roles set, via CDS-OR composition of Schnorr proofs in base h.
- Session key. Derived via HKDF-SHA256 from an ECDHE secret over ristretto255, with salt = nonce_c || nonce_s and info binding pid, eph_c, and eph_s.
- Key confirmation. HMAC-SHA256 tags over the transcript hash in both directions.
> **Note.** The v2 change from v1 is that online-auth transcripts bind to pid (per-session pseudonym), not the stable device_id. On-wire observers cannot link sessions from the same device without knowledge of device_pub.

## 4. Wire format

### 4.1 Packet header (24 bytes)

Every packet — over any transport — begins with the following fixed header. Transports MUST preserve the byte boundary of each packet.

| Offset | Size | Field | Encoding | Description |
| --- | --- | --- | --- | --- |
| 0 | 1 | version | u8 | Protocol version. MUST be 0x02 in this revision. |
| 1 | 1 | pkt_type | u8 | Packet type (§4.2). |
| 2 | 2 | flags | u16 LE | Flag bits; reserved=0 (§4.3). |
| 4 | 16 | session_id | bytes | 16-byte peer-scoped correlation id. |
| 20 | 4 | seq | u32 LE | Per-session monotonic sequence number. |
| 24 | var | payload | bytes | Packet-type-specific payload (§4.4). |

### 4.2 Packet types

| Code | Name | Direction | Description |
| --- | --- | --- | --- |
| 0x01 | HELLO | C→S | Capability / version probe (§5). |
| 0x02 | HELLO_REPLY | S→C | Negotiated version / suite / caps (§5). |
| 0x11 | SETUP_1 | C→S | Setup init. device_id, device_pub, nonce, role commitment. |
| 0x12 | SETUP_2 | S→C | Server nonce, challenge, server_pub, server proof. |
| 0x13 | SETUP_3 | C→S | Client setup proof. |
| 0x14 | SETUP_ACK | S→C | Enrollment OK (single byte 0x01). |
| 0x21 | AUTH_1 | C→S | Auth: pid, client proof, role proofs. |
| 0x22 | AUTH_2 | S→C | Server proof, nonce_s, eph_s, tag_s. |
| 0x23 | AUTH_3 | C→S | Client finished tag. |
| 0x24 | AUTH_ACK | S→C | Auth OK (single byte 0x01). |
| 0x7f | ERROR | either | u16 code + UTF-8 message (§8). |

> **Note.** Packet-type values MUST NOT be reassigned. A new message MUST be appended at an unused code; implementations that do not understand a new code MUST reply with ERROR/UnknownPacketType.

### 4.3 Flag bits

| Bit | Name | Description |
| --- | --- | --- |
| 0x0001 | FLAG_RETRANSMIT | Set by the sender when retransmitting a previously-sent packet. Receivers idempotently resend the cached response for the same (session_id, seq). |
| 0x0002…0x8000 | (reserved) | MUST be zero in this revision. |

### 4.4 Payload layouts

All payload layouts below sit immediately after the 24-byte header. All points are 32 bytes, all scalars are 32 bytes.

#### 4.4.1 HELLO / HELLO_REPLY

```text
u8   version
u16  suite_count
suite_count * u16  suite_ids (LE)
u64  caps (LE)
TLV* extensions (MIN_VERSION, MTU_HINT, VENDOR_ID, DEVICE_MODEL, …)
```

#### 4.4.2 SETUP_1

```text
u8    token_len              // 0..=128
token_len bytes pairing_token
32    device_id
32    device_pub
32    client_nonce
32    role_commitment
```

#### 4.4.3 SETUP_2

```text
32    server_nonce
16    setup_challenge
32    server_pub
32    a_s
32    s_s
```

#### 4.4.4 SETUP_3

```text
32    a_c
32    s_c
```

#### 4.4.5 AUTH_1

```text
32    pid
32    a_c                    // client Schnorr proof commitment
32    s_c                    // client Schnorr proof response
32    nonce_c
32    eph_c
32    c_prime                // rerandomized role commitment
32    rerand_a
32    rerand_s
u16   branches_len (LE)
branches_len * (32 A_i | 32 c_i | 32 s_i)
```

#### 4.4.6 AUTH_2

```text
32    server_pub
32    a_s
32    s_s
32    nonce_s
32    eph_s
32    tag_s                  // HMAC-SHA256(k_s2c, "server finished" || th)
```

#### 4.4.7 AUTH_3

```text
32    tag_c                  // HMAC-SHA256(k_c2s, "client finished" || th)
```

#### 4.4.8 ERROR

```text
u16   code (LE)             // see §8
var   utf8_message
```

### 4.5 TLV codec

Extensible messages (HELLO today; more in future) use trailing TLV lists:

```text
tlv  ::= u16 tag (LE) | u16 len (LE) | len bytes value
```

Reserved tags:

| Tag | Name | Value |
| --- | --- | --- |
| 0x0001 | MIN_VERSION | 1 byte: lowest acceptable protocol version. |
| 0x0002 | SUITE_LIST | (reserved; suite list currently in fixed field). |
| 0x0003 | CAPS | (reserved; caps currently in fixed field). |
| 0x0004 | MTU_HINT | u16 LE: path MTU hint from the sender. |
| 0x0100 | VENDOR_ID | Opaque vendor identifier (OUI or similar). |
| 0x0101 | DEVICE_MODEL | UTF-8 device model string. |

> **Note.** Unknown TLV tags MUST be ignored (skipped) so the format remains additive. Peers MUST NOT reject a packet solely because it contains an unknown tag.

### 4.6 Size limits

- MAX_DATAGRAM is 2048 bytes (header + payload). UDP senders SHOULD also respect path MTU hints carried in HELLO if smaller.
- Pairing token ≤ 128 bytes.
- TLV value ≤ 65535 bytes.

## 5. Capability negotiation

### 5.1 Purpose

HELLO / HELLO_REPLY lets peers agree on the protocol version, cipher suite, and optional feature set before starting a setup or auth flow. Deployments that fix the negotiated parameters out-of-band (e.g., fleets with a single software version) MAY skip HELLO and proceed directly to SETUP_1 or AUTH_1; the server then enforces v0x02 / suite 0x0001 / BASELINE capabilities implicitly.

### 5.2 Algorithm

Given each peer's advertised version, min_version, suite list, and caps bitmap, the negotiated parameters are:

```text
version = min(local_version, peer_version)
require  version >= max(local_min, peer_min)   else UnsupportedVersion

suite   = first in local_suites that is also in peer_suites
require  suite exists                          else UnsupportedSuite

caps    = local_caps & peer_caps
require  (caps & BASELINE) == BASELINE          else CapabilityMismatch
```

### 5.3 Capability bits

| Bit | Name | Meaning |
| --- | --- | --- |
| 1 << 0 | AUTH_V2 | Device supports online auth flow AUTH_1…AUTH_ACK. Baseline. |
| 1 << 1 | ROLE_RERAND | Supports role-commitment rerandomization proofs. Baseline. |
| 1 << 2 | ROLE_SET_MEMBERSHIP | Supports CDS-OR set-membership proof. Baseline. |
| 1 << 3 | PAIRING_TOKEN | Accepts pairing-token-gated setup. |
| 1 << 4 | TOFU_SETUP | Lab-mode TOFU pinning at first setup. |
| 1 << 8 | PROFILE_MINIMAL | Minimal profile (auth-only; setup OOB). |
| 1 << 9 | PROFILE_STANDARD | Standard profile. Baseline. |
| 1 << 10 | PROFILE_GATEWAY | Gateway / proxy profile. |
| 1 << 16 | CBOR_FRAMING | Optional CBOR-based framing variant (reserved). |

BASELINE = AUTH_V2 | ROLE_RERAND | ROLE_SET_MEMBERSHIP | PROFILE_STANDARD. Every conforming implementation MUST advertise BASELINE.

### 5.4 Negotiation errors

Negotiation failures MUST return PKT_ERROR with one of the following codes so that the initiator can localize the mismatch:

- UnsupportedVersion (0x0101): version floor could not be satisfied.
- UnsupportedSuite (0x0102): no mutually-supported cipher suite.
- CapabilityMismatch (0x0103): mutual intersection missed a BASELINE bit.

## 6. Transcript canonicalization

### 6.1 Encoding rules

Transcripts are length-prefixed, typed byte buffers. An implementation that produces byte-identical transcripts for the test-vector inputs in §10 is conformant at the transcript layer.

| Element | Encoding |
| --- | --- |
| Domain | u8 len \| bytes |
| Label | u8 len \| bytes |
| Message | u32 LE len \| bytes |
| Point | 32-byte compressed ristretto255 |
| Scalar | 32-byte canonical scalar encoding |
| u8 | 1 byte |
| u64 | 8 bytes LE |

### 6.2 Challenge and hash outputs

- Schnorr challenge scalars are produced by SHA-512 of the transcript followed by Scalar::from_bytes_mod_order_wide.
- Key-confirmation transcript hashes are produced by SHA-256 of the transcript.

### 6.3 Session pseudonym pid

The session pseudonym pid is computed as:

```text
pid = SHA256( u32_LE(len(T_PID)) || T_PID ||
              device_pub || nonce_c || eph_c || server_pub )

T_PID = b"iot-auth/pid/v1"
```

Every online-auth transcript binds to pid. This is the one byte-layout detail most likely to diverge between implementations; see the pid test vector in §10.

### 6.4 Domain separators

See Appendix A for the full list and their exact byte contents.

## 7. Cipher-suite registry and algorithm agility

### 7.1 Registered suites

| Suite ID | Curve | Hash | KDF | MAC | Status |
| --- | --- | --- | --- | --- | --- |
| 0x0001 | ristretto255 | SHA-256 | HKDF-SHA256 | HMAC-SHA256 | Mandatory |
| 0x0002 | ristretto255 | SHA-512 | HKDF-SHA512 | HMAC-SHA256 | Reserved |
| 0x0003–0x00FF | — | — | — | — | Reserved for future ECDH suites |
| 0x0100–0x01FF | — | — | — | — | Reserved for post-quantum suites |
| 0xFF00–0xFFFF | — | — | — | — | Private / experimental use |

### 7.2 Agility rules

1. Suite IDs MUST NOT be reassigned. New suites are added by appending.
1. The transcript encoding is independent of the suite. Adding a new suite does not change §6 rules.
1. Test-vector files are published per suite (see §10). A single implementation MAY support multiple suites; the one it advertises first in HELLO is preferred.

## 8. Error code registry

### 8.1 Categories

| Range | Category |
| --- | --- |
| 0x0100–0x01FF | Version / capability |
| 0x0200–0x02FF | Packet framing / parsing |
| 0x0300–0x03FF | Cryptographic validation |
| 0x0400–0x04FF | Session / replay |
| 0x0500–0x05FF | Authorization |
| 0x0600–0x06FF | Rate limiting / resource |
| 0x0700–0x07FF | Storage / backend |
| 0x7FFF | Unspecified / internal |

### 8.2 Full registry

| Code | Name | Meaning |
| --- | --- | --- |
| 0x0101 | UnsupportedVersion | No mutually-supported protocol version. |
| 0x0102 | UnsupportedSuite | No mutually-supported cipher suite. |
| 0x0103 | CapabilityMismatch | Baseline capabilities not mutually supported. |
| 0x0201 | MalformedPacket | Framing or TLV corruption. |
| 0x0202 | UnknownPacketType | pkt_type is not in the registry for this version. |
| 0x0203 | PayloadTooLarge | Exceeds MAX_DATAGRAM or declared MTU. |
| 0x0204 | PayloadTooShort | Field count / length implies more bytes than present. |
| 0x0205 | InvalidEncoding | Reserved bits set, bad UTF-8, etc. |
| 0x0301 | InvalidPoint | 32-byte ristretto decoding failed. |
| 0x0302 | NonCanonicalScalar | Scalar is not in canonical form. |
| 0x0303 | IdentityPoint | Received point is the identity. |
| 0x0304 | ProofVerifyFailed | Schnorr, rerand, or set-membership proof rejected. |
| 0x0305 | KeyConfirmFailed | HMAC key-confirmation tag mismatch. |
| 0x0306 | PeerKeyMismatch | Server public key differs from pinned value. |
| 0x0401 | UnknownSession | session_id not in the active session cache. |
| 0x0402 | SessionExpired | Session exists but its TTL has elapsed. |
| 0x0403 | ReplayDetected | (pid, nonce_c, eph_c) hash already accepted. |
| 0x0404 | SequenceOutOfOrder | seq is not the expected next value for this session. |
| 0x0501 | UnknownDevice | No enrolled device matches the presented pid. |
| 0x0502 | DeviceNotEnrolled | device_id not in registry (setup flow). |
| 0x0503 | RoleNotPermitted | Role set proof verified but role not allowed. |
| 0x0504 | PairingTokenInvalid | Pairing token missing or incorrect. |
| 0x0601 | RateLimited | Peer has exceeded the per-source failure budget. |
| 0x0602 | ServerBusy | Server declines due to transient load. |
| 0x0603 | TooManyActive | Session or response cache at capacity. |
| 0x0701 | StorageFailure | Backend I/O error. |
| 0x0702 | CredentialMissing | Expected credential (device root, server pin) absent. |
| 0x0703 | RegistryCorrupt | On-disk registry failed integrity or shape checks. |
| 0x7FFF | Unspecified | Fallback; peers SHOULD avoid emitting. |

### 8.3 Error payload

```text
u16  code (LE)
var  utf8_message         // human-readable, not machine-actionable
```

## 9. Timing, retry policy, and profiles

### 9.1 Retry policy (unreliable transports)

Over unreliable transports (UDP, CoAP/UDP, BLE with unacknowledged writes), the initiator retransmits the request with exponential backoff until it receives the expected response with matching (session_id, seq) or exceeds the retry budget.

```text
timeout(attempt) = retransmit_timeout << min(attempt, max_backoff_shift)
attempts = 0 .. max_retries
```

### 9.2 Reliable transports

Over reliable transports (TCP, QUIC, BLE-ATT with confirmations), the state machines send once and block for the response up to io_timeout. A stray packet with mismatched (session_id, seq) is discarded and the receiver keeps waiting for the expected one.

### 9.3 Profiles

A profile bundles timing and resource limits. Peers announce their profile via capability bits (§5.3); the server SHOULD apply the smaller of the two profiles' limits when responding to a given device.

| Parameter | Minimal | Standard | Gateway |
| --- | --- | --- | --- |
| retransmit_timeout | 800 ms | 800 ms | 800 ms |
| max_retries | 2 | 4 | 4 |
| max_backoff_shift | 3 | 3 | 3 |
| io_timeout | 5 s | 5 s | 5 s |
| session_ttl | 15 s | 15 s | 15 s |
| max_active_sessions | 8 | 1024 | 8192 |
| max_cached_responses | 16 | 2048 | 16384 |
| Setup supported | no (OOB) | yes | yes |

### 9.4 Idempotency on unreliable transports

Servers MUST maintain a short-TTL cache of the last response produced for each (session_id, seq). A received packet with FLAG_RETRANSMIT set, matching (session_id, seq), MUST cause the cached response to be re-emitted verbatim without re-executing the protocol step. This makes retries safe under arbitrary datagram reordering.

## 10. Test vectors and conformance

### 10.1 Deterministic harness

The reference implementation publishes a JSON test-vector file for each suite under test-vectors/<suite-id>/. Vectors are generated from a seeded RNG (deterministic) covering:

- transcripts.json — domain + fields → transcript bytes → challenge scalar.
- pid.json — (device_pub, nonce_c, eph_c, server_pub) → 32-byte pid.
- schnorr_setup.json — inputs, proof (a, s), and verification result.
- role_set.json — allowed roles, commitment, proof branches, verification result.
- kdf.json — HKDF inputs → 32-byte session key.
- kc.json — KC transcript inputs → 32-byte hash and HMAC tags.

### 10.2 JSON schema

Each vector is a JSON object with these keys:

```text
{
  "suite": "0x0001",
  "name": "pid_basic",
  "inputs": { … fields as hex strings … },
  "expected": { … outputs as hex strings … },
  "notes": "optional prose"
}
```

### 10.3 Required conformance outputs

An implementation is conformant at the cryptographic layer if, for every vector in the published suite-0x0001 corpus, it reproduces the expected outputs exactly and accepts / rejects the expected-verification cases.

## 11. Conformance checklist

An implementation claiming conformance to ZK-ARCHE v2 MUST satisfy every item below.

### 11.1 Wire format

- 24-byte header with exact field offsets from §4.1.
- Every packet type (§4.2) is recognized; unknown codes elicit ERROR/UnknownPacketType.
- All flag bits except FLAG_RETRANSMIT are zero.
- TLV parser ignores unknown tags (§4.5).

### 11.2 Cryptography

- Rejects identity point in every received point (Schnorr A, eph_c, eph_s, C, C′, server_pub, device_pub).
- Rejects non-canonical scalars.
- Produces byte-identical transcripts on the published test vectors (§10).
- Verifies all six proof types: client-setup, server-setup, client-auth, server-auth, rerand, role-set.

### 11.3 Negotiation

- Advertises BASELINE in HELLO caps (§5.3).
- Replies with the correct specific error code on version / suite / capability mismatch.

### 11.4 Session handling

- Maintains a replay cache keyed on H(pid || nonce_c || eph_c).
- Caches per-(session_id, seq) responses for at least session_ttl and replays them verbatim on retransmit.
- Zeroes the device scalar and ephemeral secret after use.

### 11.5 Errors

- Emits PKT_ERROR with a specific code from §8 for every defined failure condition.
- Does not emit error payloads containing secret state or user-supplied data.

## Appendix A. Domain separators

All domain separators are ASCII bytes without trailing nul. They are length-prefixed in transcripts per §6.1.

| Constant | Value (ASCII) | Scope |
| --- | --- | --- |
| T_SETUP | setup_client_schnorr_v1 | Client Schnorr proof during enrollment. |
| T_SETUP_SERVER | setup_server_schnorr_v1 | Server Schnorr proof during enrollment. |
| T_SERVER | server_schnorr_v1 | Server Schnorr proof during online auth. |
| T_PID | iot-auth/pid/v1 | PID derivation (§6.3). |
| T_CLIENT_V2 | client_schnorr_v2 | Client Schnorr proof during online auth. |
| T_KC_V2 | kc_v2 | Key-confirmation transcript hash. |
| T_ROLE_SET | client_role_set_v1 | CDS-OR role set-membership proof. |
| T_ROLE_RERAND | client_role_rerand_v1 | Role-commitment rerandomization proof. |

## Appendix B. Change log

- v0.2.0 (this revision): introduced transport abstraction, stable 24-byte packet header (previously 22 bytes, session_id 16B + pkt_type + version + seq), structured 16-bit error codes, cipher-suite registry, HELLO negotiation, TLV extension codec, and conformance checklist.
- v0.1.0: original ZK-ARCHE v2 reference implementation. Auth transcripts bound to pid; role commitment rerandomization; CDS-OR role-set proof; HMAC-SHA256 key confirmation.
