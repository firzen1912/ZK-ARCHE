#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

# DATA release is valid only while both the release-specific authorization
# decision and the retained secure-association authority remain current. This
# qualification surface deliberately reuses the two authoritative classifiers
# instead of creating a third lifecycle authority.

echo "[data-release-retained-authority] Rust association admission"
(
  cd rust
  cargo test -p proto association_admission::tests::canonical_corpus_matches_classifier -- --exact
)

echo "[data-release-retained-authority] Rust DATA release authorization"
(
  cd rust
  cargo test -p proto data_release_authorization::tests::canonical_v4_corpus_matches_classifier -- --exact
)

# Qualify the temporal composition, not only the two classifiers in isolation:
# establish a retained association, permit one protected release, invalidate an
# authoritative lifecycle fact, require association FAIL_CLOSED, then require
# the next DATA decision to remain non-RELEASE.  Both implementations own this
# executable sequence so a retained channel cannot outlive current authority.
echo "[data-release-retained-authority] Rust retained-association temporal lifecycle"
(
  cd rust
  cargo test -p proto --test data_release_retained_association_temporal
)

echo "[data-release-retained-authority] C association admission + DATA release authorization + retained temporal lifecycle"
make -C c \
  build/tests/test_association_admission \
  build/tests/test_data_release_authorization \
  build/tests/test_data_release_retained_association_temporal
./c/build/tests/test_association_admission
./c/build/tests/test_data_release_authorization
./c/build/tests/test_data_release_retained_association_temporal

# Guard the cross-layer fail-closed facts that DATA release depends on. The
# association corpus owns restart/replay/usage continuity; the DATA-release
# corpus owns release authority, authorization generation, revocation,
# lineage, channel binding, replay, and rollback. Any disappearance here means
# the composition is no longer qualification-complete enough to claim that a
# retained channel can continue releasing protected data safely.
association_vector="rust/test-vectors/state/association-admission-v4.txt"
data_vector="rust/test-vectors/state/data-release-authorization-v4.txt"

for required_case in \
  'case=authorization-generation-stale|' \
  'case=revocation-stale|' \
  'case=revoked|' \
  'case=lineage-stale|' \
  'case=replay-continuity-stale|' \
  'case=restart-continuity-stale|' \
  'case=usage-counter-continuity-stale|' \
  'case=binding-invalid|' \
  'case=rollback|'
do
  grep -Fq "$required_case" "$association_vector" || {
    echo "missing retained-association negative: $required_case" >&2
    exit 1
  }
done

for required_case in \
  'case=device-authority-missing|' \
  'case=device-authority-stale|' \
  'case=authorization-generation-unbound|' \
  'case=authorization-generation-stale|' \
  'case=revocation-stale|' \
  'case=revoked|' \
  'case=lineage-stale|' \
  'case=binding-invalid|' \
  'case=release-replay|' \
  'case=rollback|'
do
  grep -Fq "$required_case" "$data_vector" || {
    echo "missing DATA-release negative: $required_case" >&2
    exit 1
  }
done

echo "data-release retained-authority qualification: pass temporal_lifecycle=pass"
