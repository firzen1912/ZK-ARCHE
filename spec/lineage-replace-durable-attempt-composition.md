# LINEAGE_REPLACE durable-attempt composition contract

Status: bounded zk213 qualification contract; wire-neutral and storage-neutral.

A restart MUST NOT treat a structurally valid persisted replacement record as sufficient evidence that a replacement attempt completed. Successor resumption requires both (1) a `CONVERGED` attempt decision and (2) a structurally valid, freshness-current committed-successor recovery observation. If either axis fails, successor use is fail closed.

A clean predecessor may resume when its structurally valid record is freshness-current even when an interrupted replacement attempt was awaiting confirmation. This does not resurrect or continue the interrupted attempt; all volatile confirmation evidence is discarded and any future replacement requires a new explicit lifecycle decision.

Old authenticated snapshots (`record_generation < trusted_high_water_generation`), record-ahead inconsistencies, partial replacement state, attempt-ID mismatch, predecessor-generation mismatch, context mismatch, or missing bilateral confirmation MUST NOT activate a successor.

This composition allocates no wire field, cryptographic primitive, storage mechanism, timeout, retry policy, or collision winner. It does not establish target rollback resistance or crash-safe persistence. Normal AUTH remains NO-LEARNING.

The canonical cross-language corpus is `rust/test-vectors/replay/lineage-replace-durable-attempt-v1.txt`.
