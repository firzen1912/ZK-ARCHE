#include <stddef.h>
#include "auth/lineage_replace_storage_capability.h"

lineage_replace_storage_capability_decision_t lineage_replace_classify_storage_capability(
    const lineage_replace_storage_capability_t *capability) {
    if (capability == NULL || !capability->durable_commit_confirmed) {
        return LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_DURABILITY;
    }
    if (!capability->power_loss_recovery_supported) {
        return LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_POWER_LOSS_RECOVERY;
    }
    if (!capability->record_integrity_protected) {
        return LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_RECORD_INTEGRITY;
    }
    if (!capability->replay_protection_supported) {
        return LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_REPLAY_PROTECTION;
    }
    if (!capability->freshness_anchor_available) {
        return LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_FRESHNESS_ANCHOR;
    }
    if (!capability->freshness_anchor_integrity_valid) {
        return LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_FRESHNESS_INTEGRITY;
    }
    if (!capability->freshness_anchor_lineage_bound) {
        return LINEAGE_REPLACE_STORAGE_CAPABILITY_REJECT_FRESHNESS_BINDING;
    }
    return LINEAGE_REPLACE_STORAGE_CAPABILITY_QUALIFIED;
}
