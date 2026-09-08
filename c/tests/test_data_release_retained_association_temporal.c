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
        data_release_facts_t release = current_release();
        data_release_decision_t replay;
        assert(data_release_authorization_classify(&release).action ==
               DATA_RELEASE_ACTION_RELEASE);
        release.release_operation_unused = false;
        replay = data_release_authorization_classify(&release);
        assert(replay.action == DATA_RELEASE_ACTION_DENY);
        assert(replay.reason == DATA_RELEASE_REASON_RELEASE_REPLAY_DETECTED);
    }

    puts("DATA retained-association temporal qualification: ok mutations=7 lineage_successor=1 replay=1");
    return EXIT_SUCCESS;
}
