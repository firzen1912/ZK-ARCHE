#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
TEST_ROOT="$(mktemp -d)"
trap 'rm -rf "$TEST_ROOT"' EXIT

mkdir -p "$TEST_ROOT/.githooks" "$TEST_ROOT/scripts"
cp "$ROOT/.githooks/pre-push" "$TEST_ROOT/.githooks/pre-push"

git -C "$TEST_ROOT" init --quiet
git -C "$TEST_ROOT" config user.name "ZK-ARCHE hook test"
git -C "$TEST_ROOT" config user.email "hook-test@example.invalid"
printf 'hook test\n' > "$TEST_ROOT/tracked"
git -C "$TEST_ROOT" add tracked
git -C "$TEST_ROOT" commit --quiet -m "test fixture"

make_stub() {
  local path="$1"
  local marker="$2"
  printf '#!/usr/bin/env bash\nprintf "%%s\\n" %q >> %q\n' \
    "$marker" "$TEST_ROOT/invocations" > "$path"
  chmod +x "$path"
}

make_stub "$TEST_ROOT/scripts/ci-all.sh" "core"
make_stub "$TEST_ROOT/scripts/ci-release-qualification.sh" "release"

run_hook() {
  local update="$1"
  : > "$TEST_ROOT/invocations"
  printf '%s\n' "$update" | (cd "$TEST_ROOT" && ./.githooks/pre-push origin example.invalid)
}

assert_invocations() {
  local expected="$1"
  local actual
  actual="$(paste -sd, "$TEST_ROOT/invocations")"
  if [ "$actual" != "$expected" ]; then
    echo "expected hook invocations '$expected', got '$actual'" >&2
    exit 1
  fi
}

OID="$(git -C "$TEST_ROOT" rev-parse HEAD)"
STALE="1111111111111111111111111111111111111111"
ZERO="0000000000000000000000000000000000000000"

run_hook "refs/heads/topic $OID refs/heads/main $ZERO"
assert_invocations "release"

run_hook "refs/heads/dev $OID refs/heads/dev $ZERO"
assert_invocations "core"

run_hook "refs/heads/topic $OID refs/heads/topic $ZERO"
assert_invocations ""

: > "$TEST_ROOT/invocations"
if printf '%s\n' "refs/heads/topic $STALE refs/heads/main $ZERO" \
  | (cd "$TEST_ROOT" && ./.githooks/pre-push origin example.invalid); then
  echo "pre-push accepted a non-HEAD revision for main" >&2
  exit 1
fi
assert_invocations ""

run_hook "refs/heads/dev $OID refs/heads/dev $ZERO
refs/heads/topic $OID refs/heads/main $ZERO"
assert_invocations "release"

echo "git hook routing tests: PASS"
