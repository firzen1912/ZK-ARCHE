# Formal release gate checkpoint — 2026-08-31

Status: **QUALIFICATION GATE HARDENED; NO NEW FORMAL RESULT CLAIMED**.

This checkpoint repairs a repository-owned qualification weakness discovered while advancing TD-003. The prior release-qualification shell script synchronized formal mirrors, but if ProVerif was absent it printed a skip message and could still end with `release qualification: PASS`. When ProVerif was present it executed the historical AUTH skeleton rather than the current AUTH-v3, replay-continuity, and LINEAGE_REPLACE commit-order models.

That behavior was inconsistent with the repository evidence contract: an unavailable formal lane may be reported as unavailable, but it must not be silently converted into a passing release-qualification result, and edited/current models must not inherit results from older model text.

## New repository-owned formal gate

`scripts/ci-formal.sh` now owns local formal qualification for the currently governed ProVerif models.

It performs, in order:

1. fail-closed availability detection for `proverif`; missing ProVerif exits with status 125 and `UNAVAILABLE` rather than PASS;
2. `scripts/sync-formal-models.sh --check` before any theorem run;
3. exact execution of the canonical Rust-side synchronized models:
   - `zk_arche_auth_v3_draft.pv` — expected 9 true queries;
   - `zk_arche_replay_continuity_draft.pv` — expected 9 true queries;
   - `zk_arche_lineage_replace_commit_draft.pv` — expected 6 true queries;
4. retained textual output under `evidence/formal/` by default;
5. rejection of non-zero ProVerif execution, any `RESULT` reported false/unproved, or an unexpected count of true result lines.

The expected query counts are pinned to the current assurance contracts. A model edit that changes query inventory therefore requires a reviewed gate update rather than silently changing the evidence denominator.

## Release qualification integration

`scripts/ci-release-qualification.sh` now invokes `scripts/ci-formal.sh` as a required step. It no longer skips ProVerif and later reports overall PASS, and it no longer runs the historical AUTH skeleton as the release formal authority.

This does not mean every development machine must have ProVerif. In an environment where ProVerif is unavailable, the formal lane is truthfully **UNAVAILABLE** and full release qualification cannot be claimed. Development work may still report executable local lanes separately according to the roadmap policy.

## Evidence and claim boundary

This change advances qualification discipline and TD-003 traceability, but this automation environment still could not clone the repository because direct `github.com` DNS resolution failed. ProVerif was not executed here. Therefore:

- the new scripts are **IMPLEMENTED**;
- shell syntax was checked for the modified/new Bash scripts;
- no new ProVerif theorem result is claimed;
- the LINEAGE_REPLACE model remains **MODELED + IMPLEMENTATION-TRACEABLE**, not newly FORMALLY ANALYZED;
- prior retained AUTH-v3 and replay-continuity results remain scoped to their exact retained model blobs/runs;
- TD-001, TD-002, TD-003, and TD-004 remain open;
- no cryptographic-review, constrained-target, RFC-class, Common Contract, or deployment claim is advanced.

## Required next evidence

On an exact clean `dev` checkout with ProVerif available, run:

```text
bash scripts/ci-formal.sh
bash scripts/ci-release-qualification.sh
```

Retain exact repository HEAD, model blob SHAs, ProVerif version, complete query output, and any counterexample. Any false/unproved query or parser/tool failure is a qualification failure, not a skippable advisory.
