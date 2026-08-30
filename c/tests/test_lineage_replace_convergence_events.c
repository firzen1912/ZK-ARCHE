#include "auth/lineage_replace_convergence.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EVENT_VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-convergence-events-v1.txt"

typedef struct {
    bool local_authorized;
    bool peer_authorized;
    unsigned local_successor;
    unsigned peer_successor;
    bool local_successor_set;
    bool peer_successor_set;
    bool local_confirmed;
    bool peer_confirmed;
    bool successor_conflict;
} event_state_t;

static lineage_replace_convergence_decision_t expected(const char *value) {
    assert(value != NULL);
    if (strcmp(value, "CONVERGED") == 0) return LINEAGE_REPLACE_CONVERGED;
    if (strcmp(value, "AWAITING_CONFIRMATION") == 0)
        return LINEAGE_REPLACE_AWAITING_CONFIRMATION;
    if (strcmp(value, "UNAUTHORIZED") == 0) return LINEAGE_REPLACE_UNAUTHORIZED;
    assert(strcmp(value, "SUCCESSOR_CONFLICT") == 0);
    return LINEAGE_REPLACE_SUCCESSOR_CONFLICT;
}

static unsigned event_successor(const char *event) {
    char *end = NULL;
    unsigned long value = strtoul(event + 2, &end, 10);
    assert(end != event + 2 && *end == '\0' && value <= 0xffffffffUL);
    return (unsigned)value;
}

static void observe_successor(unsigned *slot, bool *slot_set, bool *conflict, unsigned value) {
    if (*slot_set && *slot != value) *conflict = true;
    if (!*slot_set) {
        *slot = value;
        *slot_set = true;
    }
}

static void apply_event(event_state_t *state, const char *event) {
    unsigned value;
    assert(state != NULL && event != NULL);
    if (strcmp(event, "LA") == 0) {
        state->local_authorized = true;
        return;
    }
    if (strcmp(event, "PA") == 0) {
        state->peer_authorized = true;
        return;
    }
    if (strcmp(event, "CL") == 0) {
        state->local_confirmed = false;
        return;
    }
    if (strcmp(event, "CP") == 0) {
        state->peer_confirmed = false;
        return;
    }

    value = event_successor(event);
    if (strncmp(event, "LS", 2u) == 0) {
        observe_successor(&state->local_successor, &state->local_successor_set,
                          &state->successor_conflict, value);
        return;
    }
    if (strncmp(event, "PS", 2u) == 0) {
        observe_successor(&state->peer_successor, &state->peer_successor_set,
                          &state->successor_conflict, value);
        return;
    }
    if (strncmp(event, "LC", 2u) == 0) {
        if (state->local_successor_set && state->local_successor == value)
            state->local_confirmed = true;
        return;
    }
    assert(strncmp(event, "PC", 2u) == 0);
    if (state->peer_successor_set && state->peer_successor == value)
        state->peer_confirmed = true;
}

static lineage_replace_convergence_decision_t decision(const event_state_t *state) {
    lineage_replace_convergence_facts_t facts;
    assert(state != NULL);
    facts = (lineage_replace_convergence_facts_t){
        state->local_authorized,
        state->peer_authorized,
        state->local_successor_set && state->peer_successor_set && !state->successor_conflict &&
            state->local_successor == state->peer_successor,
        state->local_confirmed,
        state->peer_confirmed};
    return lineage_replace_classify_convergence(&facts);
}

int main(void) {
    FILE *fp = fopen(EVENT_VECTOR_PATH, "r");
    char line[512];
    unsigned case_count = 0u;
    int saw_version = 0;
    assert(fp != NULL);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *name, *events, *want, *extra, *event;
        event_state_t state = {0};

        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=1") == 0) {
            saw_version = 1;
            continue;
        }
        if (strncmp(line, "case=", 5u) != 0) continue;

        name = strtok(line + 5u, "|");
        events = strtok(NULL, "|");
        want = strtok(NULL, "|");
        extra = strtok(NULL, "|");
        assert(name != NULL && events != NULL && want != NULL && extra == NULL);

        event = strtok(events, ",");
        while (event != NULL) {
            apply_event(&state, event);
            event = strtok(NULL, ",");
        }
        assert(decision(&state) == expected(want));
        case_count += 1u;
    }

    fclose(fp);
    assert(saw_version == 1);
    assert(case_count == 10u);
    puts("lineage-replace convergence event corpus: ok");
    return EXIT_SUCCESS;
}
