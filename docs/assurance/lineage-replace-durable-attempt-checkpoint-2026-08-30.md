# zk213 checkpoint — durable attempt/recovery composition

Scope: deterministic composition evidence joining the existing replacement-attempt decision, durable freshness classifier, and restart recovery classifier.

Evidence added: one shared ten-case Rust/C corpus covering current committed successor, old-valid snapshot rollback, record-ahead inconsistency, missing bilateral confirmation, attempt-ID/predecessor/context mismatch, structurally partial successor state, clean current predecessor, and interrupted attempt returning only to the current predecessor.

Security boundary: a successor is operable only when attempt convergence and durable recovery/freshness independently succeed. A clean predecessor can survive an interrupted attempt, but the interrupted attempt itself is not resumed and confirmation evidence is not learned from storage. No target storage, monotonic-counter, power-loss, cryptographic key-confirmation, wire-format, external-review, or deployment claim is made.

Roadmap effect: progress within zk213 only; no phase exit is claimed. TD-001 through TD-004 remain open as applicable.
