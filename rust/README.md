# IoT-Auth ZK-ARCHE v2 — transport-agnostic refactor

This workspace refactors the ZK-ARCHE v2 IoT authentication protocol so the
same code drives it over UDP, TCP, CoAP, BLE, or any future transport, with
crisp storage and error abstractions and a formal wire spec.

## Layout

```
iot-auth-refactor/
├── Cargo.toml                   (workspace)
├── spec/
│   ├── build_spec.js            (docx generator)
│   └── iot-auth-wire-spec.docx  (the spec)
├── test-vectors/0x0001/         (deterministic JSON vectors)
│   ├── transcript.json
│   ├── pid.json
│   ├── schnorr_auth_client.json
│   ├── role_set_membership.json
│   ├── rerandomization.json
│   └── kdf_kc.json
└── crates/
    ├── proto/          (protocol library)
    │   ├── src/
    │   │   ├── lib.rs
    │   │   ├── error.rs         (29 stable wire error codes)
    │   │   ├── caps.rs          (version, suite IDs, capability bits)
    │   │   ├── transcript.rs    (byte-stable transcript builder)
    │   │   ├── crypto.rs        (Schnorr, rerand, CDS-OR, HKDF, HMAC, Blake2b)
    │   │   ├── wire.rs          (24-byte header, PKT codes, TLV codec)
    │   │   ├── profile.rs       (Minimal / Standard / Gateway)
    │   │   ├── transport/
    │   │   │   ├── mod.rs       (Transport + ClientTransport traits)
    │   │   │   ├── udp.rs       (UDP binding)
    │   │   │   └── tcp.rs       (TCP binding, length-prefix framing)
    │   │   ├── store/
    │   │   │   ├── mod.rs       (CredentialStore, RegistryStore, ReplayCache)
    │   │   │   └── fs.rs        (filesystem backend; 0600 perms)
    │   │   └── proto/
    │   │       ├── mod.rs
    │   │       ├── common.rs    (shared send/retry helper)
    │   │       ├── payloads.rs  (SETUP_1..3, AUTH_1..3 codecs)
    │   │       ├── setup.rs     (setup state machine — transport-agnostic)
    │   │       └── auth.rs      (auth state machine — transport-agnostic)
    │   └── examples/
    │       └── gen_test_vectors.rs
    ├── client/         (binary)
    └── server/         (binary)
```

## Quickstart

### Build and test

```bash
cargo build --workspace --all-features
cargo test  --workspace --all-features     # 11 tests pass
```

### Regenerate test vectors

```bash
cargo run --example gen_test_vectors --features test-vectors
# writes test-vectors/0x0001/*.json
```

### End-to-end: run server + client over UDP

```bash
# Terminal 1
cargo run -p server -- --bind 0.0.0.0:4000 --state-dir ./server-state

# Terminal 2 (first time: enroll, then auth)
cargo run -p client -- \
    --server 127.0.0.1:4000 --transport udp \
    --state-dir ./client-state --setup --allow-tofu-setup
cargo run -p client -- \
    --server 127.0.0.1:4000 --transport udp \
    --state-dir ./client-state
```

### Over TCP — same binaries, same protocol, different transport

```bash
cargo run -p client -- --server 127.0.0.1:4000 --transport tcp \
    --state-dir ./client-state --setup --allow-tofu-setup
```

(Note: the reference server binary currently speaks UDP. A TCP server is a
straightforward adaptation of the existing UDP event loop — use
`transport::tcp::TcpServerListener` and call the same `handle_*` functions.
Left as an exercise so the example stays compact.)

## Key architectural differences from the original

| Concern               | Before                           | After                                              |
| --------------------- | -------------------------------- | -------------------------------------------------- |
| Transport             | Two ~1,200-line forks (UDP/TCP)  | One state machine + a trait                        |
| Reliability           | Coupled into application logic   | `ClientTransport::is_reliable()` selects the path  |
| Error reporting       | Free-form strings                | 29 stable `u16` wire codes, category-partitioned   |
| Negotiation           | Hard-coded protocol version      | HELLO / suite registry / capability bitmap         |
| Storage               | Direct file I/O in handlers      | `CredentialStore`, `RegistryStore`, `ReplayCache`  |
| Session idempotency   | Per-transport retry logic        | Per-(session_id, seq) response cache               |
| Profiles              | Hard-coded constants             | `Profile` struct (Minimal / Standard / Gateway)    |
| Interop artifact      | Source code only                 | Wire spec doc + JSON test vectors                  |

## Crypto unchanged

The primitives — ristretto255, SHA-256, SHA-512, HKDF-SHA256, HMAC-SHA256,
Blake2b-512 — and the transcript domain separators are **byte-identical to
v1**. An enrolled device or registry from the original code loads without
migration.

## License

Apache-2.0.
