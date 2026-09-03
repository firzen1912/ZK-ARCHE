# Raspberry Pi 5 and Jetson Orin Nano

The Pi 5 and Orin Nano use the existing C implementation directly. No board-specific cryptographic fork is required: both targets are ARM64 Linux systems capable of building the same libsodium-backed C lane used for host conformance.

## Install

On Raspberry Pi OS (64-bit) or Ubuntu/JetPack Ubuntu:

```bash
git clone https://github.com/firzen1912/ZK-ARCHE.git
cd ZK-ARCHE
git checkout feat/hardware-platforms
sudo ./platforms/linux/install.sh
```

The installer:

1. installs the native compiler, `pkg-config`, `libsodium-dev`, Python and pyserial;
2. rebuilds the C lane from source;
3. runs the C test suite before installation;
4. installs `zk-arche-server` and `zk-arche-client`;
5. installs `zk-arche-serial-bridge` for transparent STM32 UART transport;
6. installs a hardened systemd service;
7. enables the service but does **not** start it.

The default bind is `127.0.0.1:4040`, intentionally preventing accidental LAN exposure.

## Isolated hardware test LAN

Edit `/etc/zk-arche/server.env`:

```text
ZK_BIND=0.0.0.0:4040
ZK_TRANSPORT=udp
ZK_STATE_DIR=/var/lib/zk-arche
```

Apply the network policy appropriate to your test segment, then:

```bash
sudo systemctl restart zk-arche
sudo systemctl status zk-arche
```

For qualification, prefer a physically or logically isolated test LAN without Internet, CA, cloud identity, online registry, or gateway approval in the authentication path.

## Manual invocation

Server:

```bash
./c/build/auth_server \
  --bind 0.0.0.0:4040 \
  --transport udp \
  --state-dir ./server-state
```

First client enrollment on another Linux peer:

```bash
./c/build/auth_client \
  --server SERVER_IP:4040 \
  --transport udp \
  --state-dir ./client-state \
  --setup \
  --allow-tofu-setup \
  --role 2
```

Subsequent AUTH:

```bash
./c/build/auth_client \
  --server SERVER_IP:4040 \
  --transport udp \
  --state-dir ./client-state
```

`--allow-tofu-setup` is an explicit enrollment choice, not a normal-AUTH policy. For stronger demonstrations use the repository's explicit pairing/provisioning semantics rather than treating TOFU as universal trust.

## STM32 transparent serial bridge

When the STM32 uses the UART adapter, keep the Linux ZK-ARCHE server on loopback and run:

```bash
zk-arche-serial-bridge \
  --serial /dev/ttyUSB0 \
  --baud 921600 \
  --server 127.0.0.1:4040
```

The bridge only converts the MCU's two-byte-length-prefixed UART frames to UDP datagrams and returns the raw response bytes. It does not parse or decide ZK-ARCHE identity, proof, role, trust, authorization, or session state.

## Pi <-> Orin evidence run

Run each device once as responder/server and once as initiator/client. Retain:

- exact source commit;
- OS/kernel and architecture;
- compiler and libsodium version;
- packet captures or canonical wire-byte records where appropriate;
- SETUP and AUTH result;
- restart/replay negative cases;
- latency/resource measurements;
- state directory and secure-storage assumptions.

These Linux results establish the higher-capability endpoints. They do not clear MCU evidence or TD-002 by themselves.