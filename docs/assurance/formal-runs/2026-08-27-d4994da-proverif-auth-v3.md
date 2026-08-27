# ProVerif draft AUTH v3 run — `d4994daaa3dd771cfa87b42485d9032302af82f0`

This artifact retains the first successful fail-closed ProVerif execution of the non-advertised draft AUTH v3 model introduced for ADR 0001. It is scoped formal-analysis evidence for TD-003 and the FM-02/FM-03/FM-04/FM-05/FM-09 property families. It is **not** a claim that production ZK-ARCHE v3 exists, that the custom proof is computationally proven, or that ZK-ARCHE is formally verified or RFC-class documented.

## Run identity

| Field | Value |
|---|---|
| Repository commit | `d4994daaa3dd771cfa87b42485d9032302af82f0` |
| Branch | `dev` |
| GitHub Actions run | `33074032432` / run 24 |
| Formal job | `98523450937` |
| Tool | ProVerif `2.05` |
| Draft v3 canonical model | `rust/models/proverif/zk_arche_auth_v3_draft.pv` |
| Draft v3 synchronized C mirror | `c/models/proverif/zk_arche_auth_v3_draft.pv` |
| Draft v3 model SHA-256 | `a49b316029d834a84e98aea6cde13371f229c595e75400d3b40207e1fb22deda` |
| Legacy v2 model SHA-256 | `260ae30ae8452bf9e45813e47ac08869dabbbb823838bdc31cc6a8c6200031fe` |
| Retained workflow artifact | `formal-proverif-evidence`, artifact id `9647186064` |
| Retained artifact ZIP SHA-256 | `b42c6786b71f1341af7d0a1c1af7bd237baca43e419e14f9db35651076c5686a` |
| Retained artifact size | 9472 bytes |
| Rust lane | PASS |
| C lane | PASS |
| Release qualification lane | PASS |
| Draft v3 formal lane | PASS — six RESULT lines, none false/unproved |

The formal job also reran the legacy v2 model and retained its known false correspondence results as expected-negative regression evidence. The workflow does not make v2 green by suppressing those counterexamples.

## Draft v3 query-result summary

The draft v3 model produced six query results, all reported true by ProVerif 2.05:

| Property / model correspondence | Result | Evidence boundary |
|---|---|---|
| `ServerAuth1AcceptedV3` implies `ReplayRecordedV3` for the accepted AUTH_1 context | **TRUE** | Establishes accepted-message replay-record ordering only under the model's persistent/unbounded replay-table abstraction. |
| Injective `ServerCompleteV3` implies injective matching `ClientAuth3SentV3` for the same client/server/session/security/KC context | **TRUE** | Scoped FM-02/FM-03 agreement evidence for authenticated completion in the draft v3 model. |
| Injective `ClientCompleteV3` implies injective matching `ServerCompleteV3` for the same context | **TRUE** | Scoped mutual-completion evidence; depends on the modeled authenticated server-completion MAC. |
| `ClientCompleteV3` implies matching `ServerAuth2SentV3` for the same context | **TRUE** | Scoped server-authentication/context agreement evidence. |
| `ServerCompleteV3` implies matching `ClientAuth3SentV3` for the same context | **TRUE** | Scoped client-finished acceptance evidence. |
| `ServerCompleteV3` implies `TrustedRecordPresent(client)` | **TRUE** | Scoped FM-09 evidence that successful server completion remains relative to pre-existing trust. |

The formal job reported: `Validated 6 draft AUTH v3 ProVerif RESULT lines with no false/unproved query result.`

## What changed relative to the retained v2 counterexamples

The earlier retained v2 run at `a73faa3dd842e577133ff3abb46cb1431df95010` found two material agreement gaps:

1. the outer `session_id` was not authenticated by the v2 KC context, allowing session relabeling/splicing in the symbolic model; and
2. the one-byte public `AUTH_ACK` could be synthesized by an active attacker, so client completion did not prove server acceptance of `AUTH_3`.

The draft v3 model changes those exact semantics rather than weakening the queries:

```text
security_context_v3 =
  version
  + suite
  + profile
  + selected capabilities
  + session_id
  + authorization-context hash
  + critical-extension hash
  + channel-binding hash

KC-TRANSCRIPT-v3 = security_context_v3 + AUTH cryptographic context

AUTH_3 = client Finished under directional v3 key
AUTH_ACK-v3 = server-completion MAC under dedicated completion key
```

The successful v3 correspondence results therefore provide scoped evidence that the ADR 0001 design direction removes the two specific symbolic counterexamples represented by the model.

## Model/runtime correspondence anchors

The draft model is not production protocol truth. Its corresponding implementation work is currently limited to non-advertised primitives and deterministic vectors:

Rust:

- `rust/crates/proto/src/auth_v3.rs`
- `rust/test-vectors/auth-v3/reference-primitives-v1.json`

C:

- `c/include/auth/auth_v3.h`
- `c/src/proto/auth_v3.c`
- `c/tests/test_auth_v3_reference.c`

Both implementation lanes have reproduced the same draft transcript/KDF/Finished/completion values in CI, but neither production AUTH state machine advertises or selects protocol v3 yet.

The following are therefore **not** established by this run:

- production `AUTH_1/AUTH_2/AUTH_3/AUTH_ACK-v3` interoperability;
- downgrade-resistant v2/v3 negotiation;
- real parser/state-machine handling of v3 fields;
- constrained-target CPU/RAM/flash/wire budgets for full v3 AUTH;
- replay continuity across restart, rollback, or cache eviction;
- security of the custom role-membership proof;
- constant-time behavior, RNG quality, key storage, memory safety, or side-channel resistance;
- external cryptographic review;
- field/deployment qualification.

## Replay abstraction limitation

The draft v3 model uses a persistent ProVerif table for accepted replay keys. Concrete Rust and C implementations use bounded volatile replay memory. Current executable work has aligned deterministic FIFO decisions at a matched capacity and added fresh-session/concurrent-duplicate scenarios, but production capacities remain different and both implementations lose replay state on restart.

Therefore the true draft-v3 replay result must be read as:

> Under the modeled persistent/unbounded replay-state assumption, an accepted AUTH_1 is ordered after replay recording and the modeled correspondence holds. This is not evidence that bounded runtime caches retain replay protection across eviction, restart, rollback, or an undefined replay-epoch transition.

R-004/R-009 and the 2026-08-26 research report continue to own that gap.

## Claim impact

After this retained run:

```text
DRAFT AUTH-v3 MODEL EXECUTION                 RETAINED
DRAFT AUTH-v3 MODEL/TOOL/COMMIT IDENTITY     RETAINED
FM-02 draft-v3 authenticated agreement       SCOPED TRUE IN MODEL
FM-03 draft-v3 mutual completion             SCOPED TRUE IN MODEL
FM-04 accepted-message replay-record ordering SCOPED TRUE IN MODEL
FM-05 draft-v3 bound security context         SCOPED TRUE FOR MODELED FIELDS
FM-09 pre-existing-trust relation             SCOPED TRUE IN MODEL

LEGACY AUTH-v2 COUNTEREXAMPLES                STILL RETAINED
PRODUCTION AUTH-v3                            NOT IMPLEMENTED
PRODUCTION AUTH-v3 INTEROPERABILITY           NOT ESTABLISHED
FORMALLY VERIFIED                             NOT CLAIMED
COMMON-CONFORMANT                             NOT ESTABLISHED
TD-003                                        OPEN
TD-004                                        OPEN
TD-002                                        OPEN
TD-001 INDEPENDENT CRYPTO REVIEW              OPEN
RFC-CLASS DOCUMENTED                          NOT ESTABLISHED
```

This run advances `FORMALLY ANALYZED` only for the explicitly modeled draft-v3 correspondence properties at the exact model/tool/commit above. It does not promote the repository or production protocol as a whole to `FORMALLY ANALYZED`.

## Next evidence gate

Before production AUTH v3 may be advertised or selected:

1. reconcile this retained result into the formal-model traceability contract;
2. keep the legacy v2 false results visible as regression motivation;
3. define/version the normative v3 wire grammar, registry values, state transitions, errors, and downgrade behavior;
4. implement synchronized Rust/C production state-machine support behind an explicitly non-default version gate;
5. add active-attacker negative vectors for session/context mutation, forged completion, cross-session completion, downgrade, and unsupported critical context;
6. retain exact Rust/C interoperability and formal reruns at the implementation commit;
7. separately resolve replay epoch/lifetime, restart, rollback, and production-capacity semantics before promoting Common Contract replay claims.
