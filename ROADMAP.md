# ZK-ARCHE Roadmap

The repository keeps a short root roadmap only as an entry point. Canonical planning lives under [`docs/roadmaps/`](docs/roadmaps/), following the same separation used by HIVEAS between project navigation and detailed release/improvement plans.

## Canonical plans

- [Improvement roadmap](docs/roadmaps/improvement-roadmap.md) — evidence-gated Rust/C engineering, security, interoperability, IoT, P2P, and data-sovereignty work.
- [RFC-style evolution plan](docs/roadmaps/rfc-evolution-plan.md) — specification maturity, registries, profiles, secure-channel bindings, and standards-inspired protocol discipline.
- [Research archive](docs/research/README.md) — external findings and candidate ideas before roadmap promotion.
- [Research backlog](docs/research/backlog.md) — questions and technologies that still require investigation, reproduction, benchmarking, or review.
- [Technical debt](docs/technical-debt/README.md) — known unresolved gaps that should not be hidden inside roadmap prose.

## Current focus areas

Current roadmap themes include:

- signed late-enrollment grants and delegated commissioner enrollment;
- authenticated rekey and lifecycle management;
- AUTH transcript/state-machine/replay hardening;
- stateless retry cookies and scalable private lookup;
- replay-safe session resumption;
- deterministic Rust/C interoperability and vector governance;
- constrained-device byte/RAM/CPU evidence;
- RFC-style CORE/AUTH/LINK/TRUST/BIND/ENROLL/DATA decomposition;
- EDHOC/OSCORE, TLS/mTLS, and DTLS binding research;
- optional anonymous-credential and post-quantum research tracks;
- per-device data sovereignty and bounded proof-carrying data;
- P2P zero-trust mutual authentication and constrained trust profiles.

Research findings do not become roadmap commitments automatically. Promotion should follow the workflow in [`docs/research/README.md`](docs/research/README.md), and protocol-impacting decisions should be recorded in [`docs/adr/`](docs/adr/) before normative changes are made in `spec/`.
