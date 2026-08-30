#include "auth/lineage_replace_faults.h"

lineage_replace_recovery_facts_t lineage_replace_facts_at_write_cut(lineage_replace_write_cut_t cut) {
    lineage_replace_recovery_facts_t facts = {false, false, false, false, false, false};
    facts.record_integrity_valid = true;

    switch (cut) {
        case LINEAGE_REPLACE_CUT_BEFORE_BEGIN:
            facts.predecessor_active = true;
            break;
        case LINEAGE_REPLACE_CUT_AFTER_PENDING_MARKER:
            facts.predecessor_active = true;
            facts.replacement_pending = true;
            break;
        case LINEAGE_REPLACE_CUT_AFTER_SUCCESSOR_ACTIVATION:
            facts.predecessor_active = true;
            facts.replacement_pending = true;
            facts.successor_active = true;
            break;
        case LINEAGE_REPLACE_CUT_AFTER_PREDECESSOR_RETIREMENT:
            facts.replacement_pending = true;
            facts.successor_active = true;
            facts.predecessor_retired = true;
            break;
        case LINEAGE_REPLACE_CUT_AFTER_INVALIDATIONS:
            facts.replacement_pending = true;
            facts.successor_active = true;
            facts.predecessor_retired = true;
            facts.invalidations_complete = true;
            break;
        case LINEAGE_REPLACE_CUT_AFTER_COMMIT_MARKER_CLEAR:
            facts.successor_active = true;
            facts.predecessor_retired = true;
            facts.invalidations_complete = true;
            break;
        default:
            facts.record_integrity_valid = false;
            break;
    }

    return facts;
}
