#include "auth/association_admission.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/p2p/common-contract-lifecycle-v3.txt"

static bool bit(const char *v) {
    assert(v != NULL);
    if (strcmp(v, "true") == 0)
        return true;
    assert(strcmp(v, "false") == 0);
    return false;
}

static bool known_peer(const char *v) {
    return strcmp(v, "mcu-core") == 0 || strcmp(v, "linux-edge") == 0;
}

int main(void) {
    FILE *fp = fopen(VECTOR_PATH, "r");
    char line[1400];
    unsigned cases = 0u;
    unsigned established = 0u;
    unsigned failed = 0u;
    unsigned offline_established = 0u;
    unsigned cross_class = 0u;

    assert(fp != NULL);
    while (fgets(line, sizeof line, fp) != NULL) {
        char *f[20];
        size_t i;
        association_admission_facts_t facts;
        association_admission_decision_t got;
        bool infrastructure_available;
        bool authorization_generation_bound;
        bool authorization_generation_current;
        bool restart_continuity_current;
        bool mandatory_floor_compatible;

        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '#' || strncmp(line, "XC3-", 4) != 0)
            continue;

        f[0] = strtok(line, "|");
        for (i = 1u; i < 20u; ++i)
            f[i] = strtok(NULL, "|");
        assert(f[19] != NULL && strtok(NULL, "|") == NULL);
        assert(known_peer(f[1]) && known_peer(f[2]));

        infrastructure_available = bit(f[3]);
        authorization_generation_bound = bit(f[8]);
        authorization_generation_current = bit(f[9]);
        restart_continuity_current = bit(f[14]);
        mandatory_floor_compatible = bit(f[15]);

        /* Designated initializers are mandatory here: a positional initializer
           silently shifts every fact when a lifecycle fact is added. */
        facts = (association_admission_facts_t){
            .auth_complete = bit(f[4]),
            .preexisting_trust_record = bit(f[5]),
            .authorization_present = bit(f[6]),
            .authorization_fresh = bit(f[7]),
            .authorization_generation_bound = authorization_generation_bound,
            .authorization_generation_current = authorization_generation_current,
            .revocation_current = bit(f[10]),
            .explicitly_revoked = bit(f[11]),
            .lineage_current = bit(f[12]),
            .replay_continuity_current = bit(f[13]),
            .restart_continuity_current = restart_continuity_current,
            /* The v3 P2P corpus does not yet model key-usage-counter
               continuity; CORE owns that fact today. */
            .usage_counter_continuity_current = true,
            .binding_required = bit(f[16]),
            .binding_valid = bit(f[17]),
            .rollback_suspected = false,
            .trust_mutation_requested = bit(f[18])
        };

        /* Every lifecycle fact above is decided by the CORE classifier, which is
           authoritative. Only mandatory_floor_compatible is a P2P
           common-contract fact the classifier does not own. */
        got = association_admission_classify(&facts);
        if (!mandatory_floor_compatible)
            got.action = ASSOCIATION_ADMISSION_FAIL_CLOSED;

        if (strcmp(f[19], "ESTABLISH") == 0) {
            assert(got.action == ASSOCIATION_ADMISSION_ESTABLISH);
            established++;
            if (!infrastructure_available)
                offline_established++;
        } else {
            assert(strcmp(f[19], "FAIL_CLOSED") == 0);
            assert(got.action == ASSOCIATION_ADMISSION_FAIL_CLOSED);
            failed++;
        }

        if (strcmp(f[1], f[2]) != 0)
            cross_class++;
        cases++;
    }

    fclose(fp);
    assert(cases == 21u);
    assert(established == 6u);
    assert(failed == 15u);
    assert(offline_established == 5u);
    assert(cross_class >= 17u);
    puts("p2p common-contract C lifecycle qualification v3: ok cases=21 establish=6 fail_closed=15 offline_establish=5");
    return EXIT_SUCCESS;
}
