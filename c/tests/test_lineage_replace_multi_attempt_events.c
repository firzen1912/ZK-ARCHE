#include "auth/lineage_replace_attempt.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EVENT_VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-multi-attempt-events-v1.txt"

typedef struct {
    unsigned id;
    unsigned predecessor_generation;
    unsigned successor;
    unsigned context;
} attempt_t;

typedef struct {
    bool local_authorized;
    bool peer_authorized;
    bool local_attempt_present;
    bool peer_attempt_present;
    attempt_t local_attempt;
    attempt_t peer_attempt;
    unsigned local_confirmation;
    unsigned peer_confirmation;
} multi_attempt_state_t;

typedef enum {
    MULTI_ATTEMPT_INCOMPLETE = 0,
    MULTI_ATTEMPT_CLASSIFIED
} multi_attempt_status_t;

static attempt_t attempt(unsigned id) {
    switch (id) {
        case 1u: return (attempt_t){1u, 10u, 20u, 30u};
        case 2u: return (attempt_t){2u, 10u, 21u, 30u};
        case 3u: return (attempt_t){3u, 11u, 22u, 31u};
        default: assert(false); return (attempt_t){0};
    }
}

static unsigned event_id(const char *event, size_t prefix_len) {
    char *end = NULL;
    unsigned long value = strtoul(event + prefix_len, &end, 10);
    assert(end != event + prefix_len && *end == '\0' && value <= 3ul && value >= 1ul);
    return (unsigned)value;
}

static void observe_attempt(bool *present, attempt_t *slot, unsigned *confirmation, unsigned id) {
    if (!*present || slot->id != id) {
        *slot = attempt(id);
        *present = true;
        *confirmation = 0u;
    }
}

static void apply_event(multi_attempt_state_t *state, const char *event) {
    if (strcmp(event, "LA") == 0) { state->local_authorized = true; return; }
    if (strcmp(event, "PA") == 0) { state->peer_authorized = true; return; }
    if (strcmp(event, "RL") == 0) {
        state->local_attempt_present = false;
        state->local_confirmation = 0u;
        return;
    }
    if (strcmp(event, "RP") == 0) {
        state->peer_attempt_present = false;
        state->peer_confirmation = 0u;
        return;
    }
    if (strcmp(event, "X") == 0) {
        state->local_attempt_present = false;
        state->peer_attempt_present = false;
        state->local_confirmation = 0u;
        state->peer_confirmation = 0u;
        return;
    }
    if (event[0] == 'L' && event[1] != 'C') {
        observe_attempt(&state->local_attempt_present, &state->local_attempt,
                        &state->local_confirmation, event_id(event, 1u));
        return;
    }
    if (event[0] == 'P' && event[1] != 'C') {
        observe_attempt(&state->peer_attempt_present, &state->peer_attempt,
                        &state->peer_confirmation, event_id(event, 1u));
        return;
    }
    if (strncmp(event, "LC", 2u) == 0) {
        unsigned id = event_id(event, 2u);
        if (state->local_attempt_present && state->local_attempt.id == id)
            state->local_confirmation = id;
        return;
    }
    assert(strncmp(event, "PC", 2u) == 0);
    {
        unsigned id = event_id(event, 2u);
        if (state->peer_attempt_present && state->peer_attempt.id == id)
            state->peer_confirmation = id;
    }
}

static multi_attempt_status_t decision(
    const multi_attempt_state_t *state,
    lineage_replace_attempt_decision_t *result) {
    lineage_replace_attempt_facts_t facts;
    if (!state->local_attempt_present || !state->peer_attempt_present)
        return MULTI_ATTEMPT_INCOMPLETE;
    facts = (lineage_replace_attempt_facts_t){
        state->local_authorized,
        state->peer_authorized,
        state->local_attempt.id == state->peer_attempt.id,
        state->local_attempt.predecessor_generation == state->peer_attempt.predecessor_generation,
        state->local_attempt.successor == state->peer_attempt.successor,
        state->local_attempt.context == state->peer_attempt.context,
        state->local_confirmation == state->local_attempt.id,
        state->peer_confirmation == state->peer_attempt.id,
    };
    *result = lineage_replace_classify_attempt(&facts);
    return MULTI_ATTEMPT_CLASSIFIED;
}

static void assert_expected(const multi_attempt_state_t *state, const char *expected) {
    lineage_replace_attempt_decision_t result = LINEAGE_REPLACE_ATTEMPT_UNAUTHORIZED;
    multi_attempt_status_t status = decision(state, &result);
    if (strcmp(expected, "INCOMPLETE_ATTEMPT") == 0) {
        assert(status == MULTI_ATTEMPT_INCOMPLETE);
        return;
    }
    assert(status == MULTI_ATTEMPT_CLASSIFIED);
    if (strcmp(expected, "CONVERGED") == 0) assert(result == LINEAGE_REPLACE_ATTEMPT_CONVERGED);
    else if (strcmp(expected, "AWAITING_CONFIRMATION") == 0)
        assert(result == LINEAGE_REPLACE_ATTEMPT_AWAITING_CONFIRMATION);
    else {
        assert(strcmp(expected, "ATTEMPT_ID_MISMATCH") == 0);
        assert(result == LINEAGE_REPLACE_ATTEMPT_ID_MISMATCH);
    }
}

int main(void) {
    FILE *fp = fopen(EVENT_VECTOR_PATH, "r");
    char line[768];
    unsigned case_count = 0u;
    int saw_version = 0;
    assert(fp != NULL);
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *name;
        char *events;
        char *expected;
        char *extra;
        char *event;
        multi_attempt_state_t state = {0};
        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=1") == 0) { saw_version = 1; continue; }
        if (strncmp(line, "case=", 5u) != 0) continue;
        name = strtok(line + 5u, "|");
        events = strtok(NULL, "|");
        expected = strtok(NULL, "|");
        extra = strtok(NULL, "|");
        assert(name != NULL && events != NULL && expected != NULL && extra == NULL);
        event = strtok(events, ",");
        while (event != NULL) {
            apply_event(&state, event);
            event = strtok(NULL, ",");
        }
        assert_expected(&state, expected);
        case_count += 1u;
    }
    fclose(fp);
    assert(saw_version == 1);
    assert(case_count == 14u);
    puts("lineage-replace multi-attempt event corpus: ok");
    return EXIT_SUCCESS;
}
