#!/usr/bin/env bash
set -euo pipefail

# Cross-build upstream libsodium for STM32U585/Cortex-M33.
#
# Usage:
#   ./build-libsodium.sh /path/to/libsodium /absolute/install/prefix
#
# Prerequisites:
#   arm-none-eabi-gcc and the normal autotools/build dependencies required by
#   the chosen libsodium release. Use a reviewed/pinned libsodium source tree.
#
# This helper intentionally does NOT use --enable-minimal: ZK-ARCHE currently
# requires Ristretto255, Blake2b, SHA-256/HMAC and the scalar/point APIs. Linker
# garbage collection should remove unused objects instead of silently removing
# primitives from the protocol contract.

if [[ $# -ne 2 ]]; then
  echo "usage: $0 /path/to/libsodium /absolute/install/prefix" >&2
  exit 2
fi

SRC="$(cd -- "$1" && pwd)"
PREFIX="$2"
case "$PREFIX" in
  /*) ;;
  *) echo "error: install prefix must be absolute" >&2; exit 2 ;;
esac

for tool in arm-none-eabi-gcc arm-none-eabi-ar arm-none-eabi-ranlib make; do
  command -v "$tool" >/dev/null 2>&1 || {
    echo "error: missing tool: $tool" >&2
    exit 1
  }
done

# Do not enable LTO for libsodium. Keep the initial qualification build simple,
# inspectable and compatible with the upstream embedded cross-build guidance.
export CC=arm-none-eabi-gcc
export AR=arm-none-eabi-ar
export RANLIB=arm-none-eabi-ranlib
export CFLAGS="-Os -mcpu=cortex-m33 -mthumb -ffunction-sections -fdata-sections -fno-strict-aliasing"
export CPPFLAGS=""
export LDFLAGS="--specs=nosys.specs -mcpu=cortex-m33 -mthumb -Wl,--gc-sections"

cd "$SRC"

# Release tarballs ship configure. A git checkout may require ./autogen.sh first.
if [[ ! -x ./configure ]]; then
  if [[ -x ./autogen.sh ]]; then
    ./autogen.sh
  else
    echo "error: libsodium source has neither configure nor autogen.sh" >&2
    exit 1
  fi
fi

./configure \
  --host=arm-none-eabi \
  --disable-shared \
  --enable-static \
  --prefix="$PREFIX"

make clean
make -j"${JOBS:-4}"
make install

echo
echo "libsodium installed under: $PREFIX"
echo "headers: $PREFIX/include"
echo "library: $PREFIX/lib/libsodium.a"
echo
echo "Next: add those paths to the STM32CubeIDE project and verify the"
echo "final .map file, flash usage, RAM usage, and canonical ZK-ARCHE vectors."
