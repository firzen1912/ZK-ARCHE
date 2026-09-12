#include "auth/association_admission.h"
#include "auth/data_release_authorization.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    MUTATION_AUTHORIZATION_GENERATION = 0,
    MUTATION_REVOKED,
    MUTATION_LINEAGE,
    MUTATION_RESTART_CONTINUITY,
    MUTATION_USAGE_CONTINUITY,
    MUTATION_ROLLBACK,
    MUTATION_BINDING,
    MUTATION_COUNT
} mutation_t;

static association_admission_facts_t current_association(void) {
    return (association_admission_facts_t){
        true, true, true, true, true, true, true, false,
        true, true, true, true, true, true, false, false
    };
}

static data_release_facts_t current_release(void) {
    return (data_release_facts_t){
        true, true, true, true, true, true, true, true, true, true, false,
        true, true, true, true, true, true, true, true, true, true, false
    };
}

static void apply_mutation(
    mutation_t mutation,
    association_admission_facts_t *association,
    data_release_facts_t *release) {
    assert(association);
    assert(release);
    switch (mutation) {
        case MUTATION_AUTHORIZATION_GENERATION:
            association->authorization_generation_current = false;
            release->authorization_generation_current = false;
            break;
        case MUTATION_REVOKED:
            association->explicitly_revoked = true;
            release->explicitly_revoked = true;
            break;
        case MUTATION_LINEAGE:
            association->lineage_current = false;
            release->lineage_current = false;
            break;
        case MUTATION_RESTART_CONTINUITY:
            association->restart_continuity_current = false;
            break;
        case MUTATION_USAGE_CONTINUITY:
            association->usage_counter_continuity_current = false;
            break;
        case MUTATION_ROLLBACK:
            association->rollback_suspected = true;
            release->rollback_suspected = true;
            break;
        case MUTATION_BINDING:
            association->binding_valid = false;
            release->channel_binding_valid = false;
            break;
        case MUTATION_COUNT:
            assert(!"invalid mutation");
            break;
    }
}

int main(void) {
    mutation_t mutation;

    for (mutation = MUTATION_AUTHORIZATION_GENERATION; mutation < MUTATION_COUNT;
         mutation = (mutation_t)(mutation + 1)) {
        association_admission_facts_t association = current_association();
        data_release_facts_t release = current_release();

        assert(association_admission_classify(&association).action ==
               ASSOCIATION_ADMISSION_ESTABLISH);
        assert(data_release_authorization_classify(&release).action ==
               DATA_RELEASE_ACTION_RELEASE);

        apply_mutation(mutation, &association, &release);
        assert(association_admission_classify(&association).action ==
               ASSOCIATION_ADMISSION_FAIL_CLOSED);

        /* Retained traffic keys or an open transport cannot synthesize current
         * authentication authority after association admission fails closed. */
        release.authenticated = false;
        assert(data_release_authorization_classify(&release).action !=
               DATA_RELEASE_ACTION_RELEASE);
    }

    {
        association_admission_facts_t successor_association = current_association();
        data_release_facts_t predecessor_release = current_release();
        data_release_decision_t decision;

        assert(association_admission_classify(&successor_association).action ==
               ASSOCIATION_ADMISSION_ESTABLISH);
        assert(data_release_authorization_classify(&predecessor_release).action ==
               DATA_RELEASE_ACTION_RELEASE);

        /* A valid successor AUTH/association after LINEAGE_REPLACE does not
         * make predecessor-bound DATA release authority current again. */
        predecessor_release.lineage_current = false;
        predecessor_release.authenticated = true;
        decision = data_release_authorization_classify(&predecessor_release);
        assert(decision.action == DATA_RELEASE_ACTION_DENY);
        assert(decision.reason == DATA_RELEASE_REASON_LINEAGE_STALE);
    }

    {
        data_release_facts_t stale_generation = current_release();
        data_release_facts_t revoked = current_release();
        data_release_facts_t stale_lineage = current_release();
        data_release_facts_t stale_local_authority = current_release();
        data_release_facts_t stale_authorization = current_release();
        data_release_facts_t policy_mismatch = current_release();
        data_release_facts_t consumed_operation = current_release();
        data_release_facts_t rollback = current_release();
        data_release_decision_t decision;

        /* Fresh AUTH and a newly valid channel binding must not synthesize
         * current DATA authority after any retained DATA-local fact becomes unsafe. */
        stale_generation.authorization_generation_current = false;
        stale_generation.channel_binding_valid = false;
        stale_generation.authenticated = true;
        stale_generation.channel_binding_valid = true;
        decision = data_release_authorization_classify(&stale_generation);
        assert(decision.action == DATA_RELEASE_ACTION_DENY);
        assert(decision.reason == DATA_RELEASE_REASON_AUTHORIZATION_GENERATION_STALE);

        revoked.explicitly_revoked = true;
        revoked.channel_binding_valid = false;
        revoked.authenticated = true;
        revoked.channel_binding_valid = true;
        decision = data_release_authorization_classify(&revoked);
        assert(decision.action == DATA_RELEASE_ACTION_DENY);
        assert(decision.reason == DATA_RELEASE_REASON_REVOKED);

        stale_lineage.lineage_current = false;
        stale_lineage.channel_binding_valid = false;
        stale_lineage.authenticated = true;
        stale_lineage.channel_binding_valid = true;
        decision = data_release_authorization_classify(&stale_lineage);
        assert(decision.action == DATA_RELEASE_ACTION_DENY);
        assert(decision.reason == DATA_RELEASE_REASON_LINEAGE_STALE);

        stale_local_authority.device_release_authority_current = false;
        stale_local_authority.channel_binding_valid = false;
        stale_local_authority.authenticated = true;
        stale_local_authority.channel_binding_valid = true;
        decision = data_release_authorization_classify(&stale_local_authority);
        assert(decision.action == DATA_RELEASE_ACTION_DENY);
        assert(decision.reason == DATA_RELEASE_REASON_DEVICE_RELEASE_AUTHORITY_STALE);

        stale_authorization.authorization_fresh = false;
        stale_authorization.channel_binding_valid = false;
        stale_authorization.authenticated = true;
        stale_authorization.channel_binding_valid = true;
        decision = data_release_authorization_classify(&stale_authorization);
        assert(decision.action == DATA_RELEASE_ACTION_DENY);
        assert(decision.reason == DATA_RELEASE_REASON_AUTHORIZATION_STALE);

        policy_mismatch.policy_match = false;
        policy_mismatch.channel_binding_valid = false;
        policy_mismatch.authenticated = true;
        policy_mismatch.channel_binding_valid = true;
        decision = data_release_authorization_classify(&policy_mismatch);
        assert(decision.action == DATA_RELEASE_ACTION_DENY);
        assert(decision.reason == DATA_RELEASE_REASON_POLICY_MISMATCH);

        consumed_operation.release_operation_unused = false;
        consumed_operation.channel_binding_valid = false;
        consumed_operation.authenticated = true;
        consumed_operation.channel_binding_valid = true;
        decision = data_release_authorization_classify(&consumed_operation);
        assert(decision.action == DATA_RELEASE_ACTION_DENY);
        assert(decision.reason == DATA_RELEASE_REASON_RELEASE_REPLAY_DETECTED);

        rollback.rollback_suspected = true;
        rollback.channel_binding_valid = false;
        rollback.authenticated = true;
        rollback.channel_binding_valid = true;
        decision = data_release_authorization_classify(&rollback);
        assert(decision.action == DATA_RELEASE_ACTION_DENY);
        assert(decision.reason == DATA_RELEASE_REASON_ROLLBACK_SUSPECTED);
    }

    {
        data_release_facts_t release = current_release();
        data_release_decision_t replay;
        assert(data_release_authorization_classify(&release).action ==
               DATA_RELEASE_ACTION_RELEASE);
        release.release_operation_unused = false;
        replay = data_release_authorization_classify(&release);
        assert(replay.action == DATA_RELEASE_ACTION_DENY);
        assert(replay.reason == DATA_RELEASE_REASON_RELEASE_REPLAY_DETECTED);
    }

    puts("DATA retained-association temporal qualification: ok mutations=7 lineage_successor=1 fresh_auth_rebind=8 replay=1");
    return EXIT_SUCCESS;
}
