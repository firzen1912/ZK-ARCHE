# ZK-ARCHE Registries Skeleton

Status: draft skeleton.

Registry categories to define:

| Registry | Purpose |
|---|---|
| Protocol versions | wire/protocol version negotiation |
| Cryptographic suites | primitive bundle and KDF/MAC/transcript domain separation |
| Profiles | `iot-core`, `iot-edge`, `research-only`, and future conformance labels |
| Extensions | optional negotiated features such as retry cookies, lookup hints, resumption, channel binding |
| Packet/message types | SETUP, AUTH, HELLO, retry, ACK, error, and future rekey/resume types |
| Alerts/errors | deterministic error categories without leaking unnecessary private state |
| Transport bindings | UDP, TCP, TLS-exporter-bound, DTLS-style, CoAP/OSCORE-oriented |

Rules:

- New registry values require checkpoint review when they affect security, wire compatibility, or privacy.
- Experimental values must be clearly separated from stable profile values.
- Downgrade and unknown-extension behavior must be transcript-bound and tested.
