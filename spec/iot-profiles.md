# ZK-ARCHE IoT Profile Specification Skeleton

Status: draft skeleton.

Profiles:

| Profile | Target | Constraint posture |
|---|---|---|
| `iot-core` | STM32-class and ESP32-S3-class constrained devices | bounded buffers, no mandatory heavy dependencies, UDP-safe message budget |
| `iot-edge` | Raspberry Pi-class gateways and capable Linux devices | larger registries, commissioning services, optional TLS/DTLS bindings |
| `research-only` | Jetson Orin-class edge nodes, workstations, review labs | anonymous credentials, post-quantum hybrids, alternate proof systems |

Every protocol extension must declare the minimum profile where it is allowed and must provide measured byte/RAM/CPU evidence before field-readiness claims.
