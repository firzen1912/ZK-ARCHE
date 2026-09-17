# TLS Exporter Binding Negative Reference Trace

Status: specification-grade qualification companion; this document does not claim executed interoperability.

## Purpose

This trace fixes negative decision semantics for TLS/DTLS exporter binding so independent Rust and C implementations can reject context substitution without treating a transport address as protocol identity. It is subordinate to the canonical ZK-ARCHE channel/exporter-binding specification and introduces no new wire format, exporter label, registry value, or cryptographic primitive.

## Required binding inputs

A conformant TLS/DTLS adapter MUST derive or verify channel binding from the protocol-defined exporter label and complete canonical ZK-ARCHE exporter context for the specific AUTH instance. The context MUST distinguish the AUTH instance, application/protocol context, authenticated endpoint context, and negotiated ZK-ARCHE parameters required by the controlling binding specification.

The adapter MUST NOT replace protocol identity or AUTH-instance input with a socket address, port, interface identifier, DNS name, or other routing metadata unless explicitly part of the normative authenticated context.

## Reference decisions

Begin with one TLS/DTLS connection whose exporter secret is otherwise valid and one accepted ZK-ARCHE AUTH instance. Change exactly the named input while retaining the original exporter/binding value.

| Case | Mutation | Required result |
|---|---|---|
| TB-N01 | none | ACCEPT |
| TB-N02 | AUTH-instance identifier changed | REJECT |
| TB-N03 | application/protocol context changed | REJECT |
| TB-N04 | authenticated initiator endpoint context changed | REJECT |
| TB-N05 | authenticated responder endpoint context changed | REJECT |
| TB-N06 | negotiated ZK-ARCHE profile/suite context changed | REJECT |
| TB-N07 | original binding replayed for a new AUTH instance on the same lower-layer connection | REJECT |
| TB-N08 | original binding replayed after TLS/DTLS resumption for a distinct ZK-ARCHE AUTH instance | REJECT |
| TB-N09 | socket/address changes while normative authenticated context remains unchanged | decision depends only on normative authenticated context |
| TB-N10 | routing metadata substituted for a required authenticated endpoint/context field | REJECT |

## Fail-closed ordering

1. Parse and validate the canonical exporter context.
2. Verify that it belongs to the current ZK-ARCHE AUTH instance and negotiated protocol context.
3. Obtain the TLS/DTLS exporter using the controlling label and canonical context.
4. Verify the expected channel binding using the protocol-defined representation.
5. On mismatch, terminate the bound ZK-ARCHE operation according to controlling error semantics. The implementation MUST NOT drop the mismatched field, substitute routing metadata, retry with weaker context, or treat the lower-layer connection as sufficient authentication.
6. Only successful binding may feed subsequent AUTH/authorization processing.

## Evidence boundary

TB-N01 through TB-N10 are intended to become deterministic shared Rust/C fixtures. This document establishes normative negative semantics only. It does not establish identical Rust/C context bytes, adapter execution, resumption qualification, or interoperability. Those claims require exact-head executable evidence. TLS/DTLS remains an adapter and is not made mandatory for the Common Contract.