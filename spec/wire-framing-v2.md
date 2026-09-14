# ZK-ARCHE v2 Transport-Neutral Wire Framing

Status: **normative for the deployed v2 framing layout only**. This document specifies framing behavior that is already implemented independently by the Rust and C lanes. It does not promote AUTH v3, allocate new packet types, change existing bytes, or establish that every v2 payload is RFC-class documented.

## 1. Scope and ownership

This document owns the common packet envelope shared by the stable v2 implementation. Packet-type-specific payload grammars remain owned by their corresponding specification/implementation surfaces until they are promoted into the normative package.

The envelope is transport-neutral: a transport adapter carries one complete ZK-ARCHE packet without changing its bytes. Transport addresses are not protocol identities and are not serialized into this header.

All multi-byte integer fields in this envelope use little-endian encoding.

## 2. Canonical packet grammar

A v2 packet is:

```text
packet-v2 = header-v2 || payload

header-v2 =
    version        : u8
 || packet_type    : u8
 || flags          : u16-le
 || session_id     : octet[16]
 || sequence       : u32-le
```

The header is exactly 24 octets. The payload begins at offset 24 and consumes the remainder of the packet.

| Offset | Size | Field | Encoding | v2 meaning |
|---:|---:|---|---|---|
| 0 | 1 | `version` | `u8` | selected protocol version; stable v2 is `0x02` |
| 1 | 1 | `packet_type` | `u8` | value from the packet/message-type registry |
| 2 | 2 | `flags` | `u16-le` | bit field from the header-flag registry |
| 4 | 16 | `session_id` | opaque octets | peer-scoped protocol correlation identifier |
| 20 | 4 | `sequence` | `u32-le` | per-session sequence value |
| 24 | variable | `payload` | opaque here | packet-type-specific payload |

A decoder presented with fewer than 24 octets MUST fail before exposing a decoded header. A decoded header does not by itself authorize the packet: version, packet type, flags, sequence/replay state, session state, and payload remain subject to their owning state machines.

## 3. Version boundary

Stable v2 packets use `version = 0x02`.

The shared low-level Rust/C header decoders reject values below the implementation minimum supported version, but intentionally leave selected-version dispatch to negotiation/state-machine logic. Therefore:

- successful low-level header decoding MUST NOT be treated as acceptance of an unpromoted protocol version;
- an implementation MUST execute v2 payload/transcript semantics only when v2 is the selected protocol version;
- draft AUTH-v3 parsing or reference primitives MUST NOT make `0x03` production-selectable;
- a v3-selected session MUST NOT be silently reinterpreted as v2.

This distinction preserves the existing decoder contract without turning permissive framing parse into protocol-version acceptance.

## 4. Packet-type field

The stable packet-type allocations are owned by `spec/registries.md`. The current stable values are:

```text
0x01 HELLO
0x02 HELLO_REPLY
0x11 SETUP_1
0x12 SETUP_2
0x13 SETUP_3
0x14 SETUP_ACK
0x21 AUTH_1
0x22 AUTH_2
0x23 AUTH_3
0x24 AUTH_ACK
0x7f ERROR
```

The framing layer preserves the raw `u8` value. A known numeric packet type is not sufficient for acceptance: direction, selected protocol version, state-machine position, payload grammar, and session context remain authoritative.

Packet-type numbers MUST NOT be reassigned to unrelated semantics. Version-specific payload semantics are permitted only where `spec/registries.md` explicitly defines the version distinction and the selected version makes interpretation unambiguous.

## 5. Flag field

The stable v2 flag encoding is a 16-bit little-endian field. `0x0000` means no flag and `0x0001` is the registered retransmission flag.

The low-level header decoder exposes the raw flag word. That decode step is not a license to assign semantics to unknown bits. Unassigned bits MUST NOT silently alter authentication, authorization, transcript, replay, trust, or downgrade behavior. Their protocol treatment is governed by the selected-version registry/state-machine contract.

The retransmission flag does not create a new authenticated identity and does not override replay/session admission. It only marks a retransmission for the owning protocol logic.

## 6. Session identifier and sequence

`session_id` is exactly 16 octets and is carried unchanged by the framing layer. It is a protocol correlation value, not a transport address and not, by itself, proof of peer identity or authorization.

`sequence` is encoded as an unsigned 32-bit little-endian integer. The framing layer only serializes/deserializes the value. Replay, ordering, duplicate, restart, and continuity decisions belong to the replay/session lifecycle contracts.

Changing transport address or reconnecting a transport MUST NOT implicitly change the meaning of an existing protocol `session_id` or authorize a new replay epoch.

## 7. Datagram/resource boundary

The current Rust and C packet builders share:

```text
HEADER_LEN   = 24
MAX_DATAGRAM = 2048
MAX_PAYLOAD  = 2024
```

A builder MUST NOT emit a payload larger than its configured maximum. The current production-compatible builders enforce the 2024-octet payload ceiling.

This 2048-octet ceiling is an implementation/profile constraint, not proof that every physical transport can carry a 2048-octet packet without fragmentation. Smaller path MTUs, constrained-link framing, fragmentation, or transport-specific limits remain binding-specific concerns. Any constrained profile claiming a smaller normative ceiling must define and qualify that ceiling separately.

## 8. Transport-adapter contract

A transport adapter carrying stable v2 framing:

1. MUST preserve the packet bytes and packet boundary presented to the protocol decoder;
2. MUST NOT rewrite `session_id`, `sequence`, version, flags, packet type, or payload bytes as a transport-side convenience;
3. MUST NOT treat source/destination network addresses as authenticated ZK-ARCHE identity;
4. MUST surface truncation or transport loss as transport failure rather than synthesizing protocol fields;
5. MAY impose a smaller maximum packet size, but MUST expose that constraint to the owning profile/negotiation layer rather than silently truncating a packet.

Stream transports need an outer record-boundary mechanism. That outer length/boundary mechanism is transport binding metadata and is not part of the 24-octet ZK-ARCHE header unless a future versioned binding specification says otherwise.

## 9. Rust/C implementation traceability

The deployed framing contract is independently represented by:

- Rust: `rust/crates/proto/src/wire.rs`
- C: `c/src/wire/wire.c`
- C constants/API: `c/include/auth/auth.h` and `c/include/auth/auth_wire.h`
- historical v2 interoperability description: `c/wire-spec.md`
- allocation/change-control authority: `spec/registries.md`

At the current implementation state, Rust and C agree on the 24-octet offsets, little-endian flags/sequence encoding, 16-octet session identifier, stable packet-type values, retransmission flag value, and 2048/2024 builder ceilings.

This traceability establishes an implementation-backed normative framing description. It does not establish full v2 payload grammar completion, exact-head executable qualification, transport interoperability across every adapter, constrained-target measurement, AUTH-v3 promotion, or RFC status.

## 10. Change control

A change to any of the following is wire-relevant and requires checkpoint-style review plus updated Rust/C evidence:

- header length or field order;
- integer width or endianness;
- `session_id` width;
- packet-type allocation;
- flag allocation or unknown-flag behavior;
- version-dispatch meaning;
- sequence/replay interpretation that changes acceptance;
- maximum size when claimed as a protocol/profile requirement.

Existing stable numbers and byte positions MUST NOT be silently repurposed. New behavior must use the registry/version/profile/extension machinery and retain compatibility/negative evidence appropriate to the change.
