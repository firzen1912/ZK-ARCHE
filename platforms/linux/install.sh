#!/usr/bin/env bash
set -euo pipefail

# ZK-ARCHE Linux-edge installer for Raspberry Pi 5 and Jetson Orin Nano.
# Run from a checked-out ZK-ARCHE repository:
#   sudo ./platforms/linux/install.sh
#
# The service binds loopback by default. Change /etc/zk-arche/server.env
# deliberately before exposing it to an MCU/LAN test network.

if [[ ${EUID} -ne 0 ]]; then
  echo "error: run as root (sudo $0)" >&2
  exit 1
fi

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd -- "${SCRIPT_DIR}/../.." && pwd)"

if [[ ! -f "${REPO_ROOT}/c/Makefile" ]]; then
  echo "error: could not locate ZK-ARCHE c/Makefile" >&2
  exit 1
fi

if command -v apt-get >/dev/null 2>&1; then
  export DEBIAN_FRONTEND=noninteractive
  apt-get update
  apt-get install -y --no-install-recommends \
    build-essential pkg-config libsodium-dev ca-certificates
else
  echo "error: this installer currently supports Debian/Ubuntu/Raspberry Pi OS apt systems" >&2
  exit 1
fi

# Build and run the host conformance suite before installing binaries.
make -C "${REPO_ROOT}/c" clean
make -C "${REPO_ROOT}/c" all
make -C "${REPO_ROOT}/c" test

install -D -m 0755 "${REPO_ROOT}/c/build/auth_server" /usr/local/bin/zk-arche-server
install -D -m 0755 "${REPO_ROOT}/c/build/auth_client" /usr/local/bin/zk-arche-client

install -d -m 0755 /etc/zk-arche
install -d -m 0700 /var/lib/zk-arche

if [[ ! -f /etc/zk-arche/server.env ]]; then
  cat >/etc/zk-arche/server.env <<'EOF'
# Default is deliberately loopback-only. For an isolated hardware test LAN,
# set e.g. ZK_BIND=0.0.0.0:4040 after applying appropriate network policy.
ZK_BIND=127.0.0.1:4040
ZK_TRANSPORT=udp
ZK_STATE_DIR=/var/lib/zk-arche
EOF
  chmod 0600 /etc/zk-arche/server.env
fi

cat >/etc/systemd/system/zk-arche.service <<'EOF'
[Unit]
Description=ZK-ARCHE authentication peer/server
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
EnvironmentFile=/etc/zk-arche/server.env
ExecStart=/usr/local/bin/zk-arche-server --bind ${ZK_BIND} --transport ${ZK_TRANSPORT} --state-dir ${ZK_STATE_DIR}
Restart=on-failure
RestartSec=2

# Keep the protocol process unprivileged and constrain filesystem access.
DynamicUser=yes
StateDirectory=zk-arche
StateDirectoryMode=0700
NoNewPrivileges=yes
PrivateTmp=yes
PrivateDevices=yes
ProtectSystem=strict
ProtectHome=yes
ProtectKernelTunables=yes
ProtectKernelModules=yes
ProtectKernelLogs=yes
ProtectControlGroups=yes
ProtectClock=yes
ProtectHostname=yes
RestrictSUIDSGID=yes
LockPersonality=yes
MemoryDenyWriteExecute=yes
CapabilityBoundingSet=
AmbientCapabilities=
RestrictAddressFamilies=AF_INET AF_INET6

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable zk-arche.service

echo
echo "Installed:"
echo "  /usr/local/bin/zk-arche-server"
echo "  /usr/local/bin/zk-arche-client"
echo "  /etc/zk-arche/server.env"
echo "  zk-arche.service (enabled, not started)"
echo
echo "Review /etc/zk-arche/server.env, then run:"
echo "  sudo systemctl start zk-arche"
echo "  sudo systemctl status zk-arche"
echo
echo "For a local client smoke test after enrollment configuration:"
echo "  zk-arche-client --server 127.0.0.1:4040 --transport udp --state-dir ./client-state --setup --allow-tofu-setup"
