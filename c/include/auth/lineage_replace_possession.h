#ifndef AUTH_LINEAGE_REPLACE_POSSESSION_H
#define AUTH_LINEAGE_REPLACE_POSSESSION_H
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    LINEAGE_REPLACE_POSSESSION_REJECT_CURRENT_CREDENTIAL_CONTROL = 0,
    LINEAGE_REPLACE_POSSESSION_REJECT_SUCCESSOR_KEY_CONTROL = 1,
    LINEAGE_REPLACE_POSSESSION_VERIFIED = 2
} lineage_replace_possession_decision_t;

typedef struct {
    bool verification_valid;
    uint8_t session_id[16];
    uint8_t subject_reference[32];
} lineage_replace_verified_lifecycle_possession_proof_t;

lineage_replace_possession_decision_t lineage_replace_classify_possession(
    const lineage_replace_verified_lifecycle_possession_proof_t *current_credential_proof,
    const lineage_replace_verified_lifecycle_possession_proof_t *successor_key_proof,
    const uint8_t expected_session_id[16],
    const uint8_t expected_predecessor_credential_reference[32],
    const uint8_t expected_successor_key_reference[32]);
#endif
