# ZK-ARCHE Roadmap

The canonical unified Rust/C/Python roadmap is maintained at [`docs/improvement-roadmap.md`](docs/improvement-roadmap.md).

Current focus areas added for future work:

- late enrollment through signed one-time `EnrollmentGrant` objects;
- delegated commissioner enrollment after successful `AUTH`;
- authenticated rekey and re-registration;
- AUTH transcript v3 with full security-context binding;
- strict AUTH state-machine, sequence, session, and transport validation;
- stateless `AUTH_RETRY` cookies for unauthenticated-work throttling;
- optional encrypted lookup hints for scalable server lookup without cleartext identity;
- replay-safe 1-RTT session resumption;
- AUTH metrics CI for wire size, RAM, CPU, registry scaling, replay, and mutation testing;
- optional anonymous-credential and post-quantum research tracks that are not mandatory for constrained `iot-core` targets until measured and reviewed;
- RFC-style protocol evolution toward EDHOC-like compactness, TLS/mTLS-style transcript/channel-binding rigor, and DTLS-style datagram robustness;
- Python lane integration for readable reference behavior, vector validation, and protocol experimentation;
- protocol-suite decomposition into `ZK-ARCHE-CORE`, `ZK-ARCHE-AUTH`, `ZK-ARCHE-BIND`, `ZK-ARCHE-ENROLL`, and `ZK-ARCHE-DATA`;
- per-device data sovereignty using device-local root secrets, encrypted-by-default protected data, policy-bound release, local auditability, and revocable epochs;
- ZK-minimal proof-carrying data for constrained devices, with heavyweight anonymous credentials, zkSNARKs, and post-quantum work kept gateway/research-only until measured and reviewed.

The roadmap now includes an IoT capability contract covering STM32-class, ESP32-S3-class, Raspberry Pi-class, and Jetson Orin-class deployments. Any feature that cannot fit the constrained `iot-core` profile must remain optional, negotiated, or explicitly marked gateway/research-only.

For the sovereignty roadmap, `iot-core` uses only small fixed proofs such as Schnorr possession proofs, bounded role-membership proofs, MAC/signature release tokens, hash commitments, and compact audit chains. General-purpose zkSNARK/STARK proving, large anonymous credentials, post-quantum hybrid suites, and certificate-chain-heavy modes are explicitly outside the baseline constrained profile.


The RFC-style evolution plan is maintained at [`docs/rfc-evolution-plan.md`](docs/rfc-evolution-plan.md). It is a specification-maturity roadmap, not a claim that ZK-ARCHE is already an IETF RFC.
