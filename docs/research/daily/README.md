# Daily Research Reports

Completed recurring ZK-ARCHE research reports live here using ISO dates:

```text
docs/research/daily/YYYY-MM-DD.md
```

Do not create empty reports for future dates. A file exists only when research was actually performed.

## Report template

````markdown
# ZK-ARCHE Research — YYYY-MM-DD

## Executive synthesis

- What is genuinely new?
- Why does it matter to ZK-ARCHE?
- What deserves reproduction, benchmarking, prototyping, promotion, deferral, or rejection?

## Repository context reviewed

Record the implementation/spec/roadmap state that was read before evaluating external work.

## Findings

### 1. Finding title

- **Source:**
- **Source type / venue:**
- **Publication or release date:**
- **Verified claim:**
- **Problem addressed:**
- **Evidence maturity:** concept | formal | software | hardware | deployed | externally-reviewed
- **Limitations / uncertainty:**
- **ZK-ARCHE inference:**
- **Wire / RAM / CPU / dependency impact:**
- **Rust/C / vector compatibility impact:**
- **Recommended disposition:** investigate | reproduce | benchmark | prototype | promote | defer | reject | research-only

## Cross-source synthesis

Separate corroborated conclusions from disagreements or unresolved assumptions.

## Candidate backlog updates

List proposed additions/changes to `../backlog.md`.

## Roadmap impact

```yaml
roadmap_impact:
  candidate_phase: null
  recommendation: investigate
  evidence_maturity: concept
  protocol_impact: none
  required_next_evidence: null
  roadmap_action: none
  promotion_requirement: explicit review
```

## Follow-up sources

Prioritize the primary sources that deserve deeper review next.
````

## Quality rule

The goal is not to maximize paper count. A report should spend detail on sources that can change an architecture decision, security assumption, protocol mechanism, benchmark target, implementation priority, interoperability requirement, or research hypothesis.
