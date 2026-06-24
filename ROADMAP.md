# ZK-ARCHE Roadmap

The canonical combined Rust/C roadmap is maintained at [`docs/improvement-roadmap.md`](docs/improvement-roadmap.md).

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
- optional anonymous-credential and post-quantum research tracks that are not mandatory for constrained `iot-core` targets until measured and reviewed.

The roadmap now includes an IoT capability contract covering STM32-class, ESP32-S3-class, Raspberry Pi-class, and Jetson Orin-class deployments. Any feature that cannot fit the constrained `iot-core` profile must remain optional, negotiated, or explicitly marked gateway/research-only.
