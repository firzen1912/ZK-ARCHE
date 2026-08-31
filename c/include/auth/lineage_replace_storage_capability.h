#ifndef AUTH_LINEAGE_REPLACE_STORAGE_CAPABILITY_H
#define AUTH_LINEAGE_REPLACE_STORAGE_CAPABILITY_H

#include <stdbool.h>

/* Declared adapter capabilities for the rollback-resistant lineage-replacement
 * qualification path. A true field is a claim about the concrete adapter and
 * target configuration; this classifier does not itself prove that claim. */
typedef struct {
    bool durable_commit_confirmed;
    bool power_loss_recovery_supported;
    bool record_integrity_protected;
    bool replay_protection_supported;
    bool freshness_anchor_available;
    bool freshness_anchor_integrity_valid;
    bool freshness_anchor_lineage_bound;
} lineage_replace_storage_capability_t;

typedef enum {
    LINEAGE_REPLACE_STORAGE_CAPABILITY_QUALIFIED = 0,
    LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_DURABILITY,
    LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_POWER_LOSS_RECOVERY,
    LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_RECORD_INTEGRITY,
    LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_REPLAY_PROTECTION,
    LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_FRESHNESS_ANCHOR,
    LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_FRESHNESS_INTEGRITY,
    LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_FRESHNESS_BINDING
} lineage_replace_storage_capability_decision_t;

lineage_replace_storage_capability_decision_t lineage_replace_classify_storage_capability(
    const lineage_replace_storage_capability_t *capability);

#endif
