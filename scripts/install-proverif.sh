#!/usr/bin/env bash
set -euo pipefail

PROVERIF_VERSION="2.05"
PROVERIF_SHA256="4871f53c32ab4a04669a060c4886ba5d9080496963fb980a9a62d2c429ceabc4"
PROVERIF_URL="https://proverif.inria.fr/proverif${PROVERIF_VERSION}.tar.gz"
DESTINATION="${1:?usage: install-proverif.sh DESTINATION}"
ARCHIVE="$DESTINATION/proverif${PROVERIF_VERSION}.tar.gz"
SOURCE="$DESTINATION/source"
BIN="$DESTINATION/bin"

for tool in curl sha256sum tar ocamlopt; do
  if ! command -v "$tool" >/dev/null 2>&1; then
    echo "ProVerif installer requires $tool" >&2
    exit 1
  fi
done

mkdir -p "$SOURCE" "$BIN"

if [ ! -f "$ARCHIVE" ]; then
  curl --fail --location --output "$ARCHIVE" "$PROVERIF_URL"
fi

printf '%s  %s\n' "$PROVERIF_SHA256" "$ARCHIVE" | sha256sum --check --status -
echo "ProVerif $PROVERIF_VERSION source checksum: PASS"

tar -xzf "$ARCHIVE" -C "$SOURCE" --strip-components=1
(cd "$SOURCE" && ./build -nointeract)
install -m 0755 "$SOURCE/proverif" "$BIN/proverif"

PROVERIF_BANNER="$("$BIN/proverif" -help 2>&1)"
printf '%s\n' "$PROVERIF_BANNER" | sed -n '1p'
echo "ProVerif $PROVERIF_VERSION installed in $BIN"
