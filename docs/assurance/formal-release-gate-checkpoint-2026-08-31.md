# Formal release gate checkpoint — 2026-08-31

Status: **QUALIFICATION GATE HARDENED; EXACT-EVIDENCE PROVENANCE ADDED; NO NEW FORMAL RESULT CLAIMED**.

This checkpoint repairs repository-owned qualification weaknesses discovered while advancing TD-003. The first repair made the formal lane fail closed when ProVerif is unavailable and moved release qualification from the historical AUTH skeleton to the current AUTH-v3, replay-continuity, and LINEAGE_REPLACE commit-order models. This update tightens the evidence producer further so a successful tool run cannot be detached from the exact repository/model/tool state that produced it.

That behavior is required by the formal evidence contract: an unavailable lane may be reported as unavailable, edited/current models must not inherit results from older model text, and a `FORMALLY ANALYZED` claim must be tied to an exact model blob, repository commit, tool name/version, and retained output.

## Repository-owned formal gate

`scripts/ci-formal.sh` owns local formal qualification for the currently governed ProVerif models.

It performs, in order:

1. fail-closed availability detection for `git` and `proverif`; missing prerequisites exit with status 125 and `UNAVAILABLE` rather than PASS;
2. exact repository `HEAD` resolution before theorem execution;
3. ProVerif version capture before theorem execution;
4. `scripts/sync-formal-models.sh --check` before any theorem run;
5. exact execution of the canonical Rust-side synchronized models:
   - `zk_arche_auth_v3_draft.pv` — expected 9 true queries;
   - `zk_arche_replay_continuity_draft.pv` — expected 9 true queries;
   - `zk_arche_lineage_replace_commit_draft.pv` — expected 6 true queries;
6. per-model verification that the working-tree model hashes to the same Git blob recorded at exact `HEAD`; a modified/untracked model fails qualification before ProVerif runs;
7. retained output under `evidence/formal/` with repository-HEAD-qualified filenames;
8. retention of both PASS and FAIL logs so failed/unproved runs and counterexample output are not discarded;
9. a per-run TSV manifest binding repository HEAD, ProVerif version, model name/path, exact model blob, expected and observed true-query counts, result, and log path;
10. rejection of non-zero ProVerif execution, any `RESULT` reported false/unproved, or an unexpected count of true result lines.

The expected query counts remain pinned to the current assurance contracts. A model edit that changes query inventory therefore requires a reviewed gate update rather than silently changing the evidence denominator.

The generated `evidence/formal/` artifacts remain local/generated evidence and are ignored by Git. A result becomes repository-retained evidence only when its exact manifest/log content (or a faithful reviewed transcription of it) is deliberately added under the repository assurance evidence structure. The gate itself does not silently promote a local run into a roadmap claim.

## Release qualification integration

`scripts/ci-release-qualification.sh` invokes `scripts/ci-formal.sh` as a required step. It does not skip ProVerif and later report overall PASS, and it does not use the historical AUTH skeleton as the release formal authority.

This does not mean every development machine must have ProVerif. In an environment where ProVerif is unavailable, the formal lane is truthfully **UNAVAILABLE** and full release qualification cannot be claimed. Development work may still report executable local lanes separately according to the roadmap policy.

## Validation performed for this hardening

The automation execution environment could not clone the repository because direct `github.com` DNS resolution failed, so complete exact-head repository-owned qualification remained unavailable.

The updated `ci-formal.sh` was nevertheless falsified in an isolated temporary Git repository with a fake ProVerif 2.05-compatible command surface and synchronized placeholder model pairs. That harness verified:

- Bash syntax (`bash -n`);
- exact repository HEAD capture;
- exact tracked-model Git blob capture and working-tree equality checking;
- distinct PASS log creation for all three configured model lanes;
- expected true-query count enforcement for 9/9/6 queries;
- generation of the provenance TSV manifest.

This synthetic harness validates gate mechanics only. It is **not** protocol evidence, not a ProVerif theorem result, and not a substitute for running the real repository models with ProVerif 2.05.

## Evidence and claim boundary

This change advances qualification discipline and TD-003 traceability, but ProVerif was not executable against the real exact-current repository models in this environment. Therefore:

- the provenance-aware gate is **IMPLEMENTED**;
- gate syntax/mechanics were locally **TESTED** in a synthetic harness;
- no new ProVerif theorem result is claimed;
- the LINEAGE_REPLACE model remains **MODELED + IMPLEMENTATION-TRACEABLE**, not newly FORMALLY ANALYZED;
- prior retained AUTH-v3 and replay-continuity results remain scoped to their exact retained model blobs/runs;
- TD-001, TD-002, TD-003, and TD-004 remain open;
- no cryptographic-review, constrained-target, RFC-class, Common Contract, or deployment claim is advanced.

## Required next evidence

On an exact clean `dev` checkout with ProVerif 2.05 available, run:

```text
bash scripts/ci-formal.sh
bash scripts/ci-release-qualification.sh
```

Retain the generated repository-HEAD-qualified manifest/logs and promote the exact result into `docs/assurance/formal-runs/` only after review. Any false/unproved query, query-count drift, model/HEAD mismatch, parser/tool failure, or missing prerequisite is a qualification failure/unavailable condition, never a skippable advisory or inferred PASS.
