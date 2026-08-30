# Lineage replacement convergence checkpoint — 2026-08-30

Scope: zk213 wire-neutral distributed replacement convergence.

## Reviewed surfaces

- `spec/lineage-replace-convergence-contract.md`
- Rust/C convergence classifiers
- shared `lineage-replace-convergence-v1.txt` corpus
- Rust/C corpus consumers

## Security boundary

The classifier requires mutually authorized replacement context, an identical successor lineage, and bilateral confirmation before reporting convergence. Competing authenticated successor candidates fail closed. Missing confirmation remains pending and does not authorize use of the successor. Normal AUTH remains NO-LEARNING.

## Negative evidence retained

CV-05/CV-06 reject one-sided authorization. CV-07/CV-08 reject different successor candidates even if confirmation facts are otherwise present. CV-02 through CV-04 prove that unilateral or absent confirmation cannot be interpreted as completed convergence.

## Claims not made

This checkpoint does not claim a deployed convergence protocol, reliable delivery, timeout correctness, distributed consensus, crash recovery across peers, cryptographic key-confirmation soundness, target measurements, rollback-resistant hardware, formal proof of this classifier, independent review, Common Contract conformance, RFC-class completion, or deployment qualification.
