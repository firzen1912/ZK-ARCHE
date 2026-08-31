#include "auth/lineage_replace_possession.h"
#include <string.h>

lineage_replace_possession_decision_t lineage_replace_classify_possession(
    const lineage_replace_verified_lifecycle_possession_proof_t *current_credential_proof,
    const lineage_replace_verified_lifecycle_possession_proof_t *successor_key_proof,
    const uint8_t expected_session_id[16],
    const uint8_t expected_predecessor_credential_reference[32],
    const uint8_t expected_successor_key_reference[32]) {
    if (current_credential_proof == NULL || expected_session_id == NULL ||
        expected_predecessor_credential_reference == NULL ||
        expected_successor_key_reference == NULL ||
        !current_credential_proof->verification_valid ||
        memcmp(current_credential_proof->session_id, expected_session_id, 16u) != 0 ||
        memcmp(current_credential_proof->subject_reference,
               expected_predecessor_credential_reference, 32u) != 0) {
        return LINEAGE_REPLACE_POSSESSION_REJECT_CURRENT_CREDENTIAL_CONTROL;
    }
    if (successor_key_proof == NULL || !successor_key_proof->verification_valid ||
        memcmp(successor_key_proof->session_id, expected_session_id, 16u) != 0 ||
        memcmp(successor_key_proof->subject_reference,
               expected_successor_key_reference, 32u) != 0) {
        return LINEAGE_REPLACE_POSSESSION_REJECT_SUCCESSOR_KEY_CONTROL;
    }
    return LINEAGE_REPLACE_POSSESSION_VERIFIED;
}
