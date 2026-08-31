#include "auth/lineage_replace_reconciliation_transition.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-reconciliation-provenance-v1.txt"
typedef struct { unsigned local_attempt; unsigned peer_attempt; unsigned local_confirmation; unsigned peer_confirmation; } provenance_state_t;
static unsigned event_id(const char *event, size_t n) { char *end = NULL; unsigned long value = strtoul(event + n, &end, 10); assert(end != event + n && *end == '\0' && value <= 0xffffffffUL); return (unsigned)value; }
static void observe(unsigned *attempt, unsigned *confirmation, unsigned id) { if (*attempt != id) { *attempt = id; *confirmation = 0u; } }
static void apply(provenance_state_t *state, const char *event) {
    unsigned id;
    if (strcmp(event, "RL") == 0) { state->local_attempt = 0u; state->local_confirmation = 0u; return; }
    if (strcmp(event, "RP") == 0) { state->peer_attempt = 0u; state->peer_confirmation = 0u; return; }
    if (strcmp(event, "X") == 0) { *state = (provenance_state_t){0}; return; }
    id = event_id(event, (event[1] == 'C') ? 2u : 1u);
    if (strncmp(event, "LC", 2u) == 0) { if (state->local_attempt == id) state->local_confirmation = id; return; }
    if (strncmp(event, "PC", 2u) == 0) { if (state->peer_attempt == id) state->peer_confirmation = id; return; }
    if (event[0] == 'L') { observe(&state->local_attempt, &state->local_confirmation, id); return; }
    assert(event[0] == 'P'); observe(&state->peer_attempt, &state->peer_confirmation, id);
}
static lineage_replace_attempt_evidence_decision_t evidence(const provenance_state_t *state) {
    lineage_replace_attempt_evidence_facts_t facts = {state->local_attempt, state->peer_attempt, state->local_confirmation, state->peer_confirmation};
    return lineage_replace_classify_attempt_evidence(&facts);
}
static lineage_replace_reconciliation_decision_t decision(const char *value) {
    if (strcmp(value, "PAIR_SUCCESSOR_READY") == 0) return LINEAGE_REPLACE_PAIR_SUCCESSOR_READY;
    if (strcmp(value, "PAIR_PREDECESSOR_READY") == 0) return LINEAGE_REPLACE_PAIR_PREDECESSOR_READY;
    if (strcmp(value, "RECONCILIATION_REQUIRED") == 0) return LINEAGE_REPLACE_RECONCILIATION_REQUIRED;
    if (strcmp(value, "PAIR_CONTINUITY_BROKEN") == 0) return LINEAGE_REPLACE_PAIR_CONTINUITY_BROKEN;
    assert(strcmp(value, "SUCCESSOR_DIVERGENCE") == 0); return LINEAGE_REPLACE_SUCCESSOR_DIVERGENCE;
}
static lineage_replace_reconciliation_transition_t transition(const char *value) {
    if (strcmp(value, "HOLD") == 0) return LINEAGE_REPLACE_RECONCILIATION_HOLD;
    if (strcmp(value, "ACTIVATE_SUCCESSOR") == 0) return LINEAGE_REPLACE_RECONCILIATION_ACTIVATE_SUCCESSOR;
    if (strcmp(value, "RESUME_PREDECESSOR") == 0) return LINEAGE_REPLACE_RECONCILIATION_RESUME_PREDECESSOR;
    if (strcmp(value, "REJECT_DIVERGENCE") == 0) return LINEAGE_REPLACE_RECONCILIATION_REJECT_DIVERGENCE;
    assert(strcmp(value, "CONTINUITY_BROKEN") == 0); return LINEAGE_REPLACE_RECONCILIATION_CONTINUITY_BROKEN;
}
int main(void) {
    FILE *fp = fopen(VECTOR_PATH, "r"); char line[768]; unsigned cases = 0u; int version = 0; assert(fp != NULL);
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *fields[6] = {0}; char *cursor; char *event; unsigned i; provenance_state_t state = {0}; lineage_replace_reconciliation_transition_facts_t facts;
        line[strcspn(line, "\r\n")] = '\0'; if (strcmp(line, "version=1") == 0) { version = 1; continue; } if (strncmp(line, "case=", 5u) != 0) continue;
        cursor = strtok(line + 5u, "|"); for (i = 0u; i < 6u && cursor != NULL; i += 1u) { fields[i] = cursor; cursor = strtok(NULL, "|"); } assert(i == 6u && cursor == NULL);
        event = strtok(fields[3], ","); while (event != NULL) { apply(&state, event); event = strtok(NULL, ","); }
        facts = (lineage_replace_reconciliation_transition_facts_t){ decision(fields[1]), decision(fields[2]), evidence(&state), strcmp(fields[4], "1") == 0 };
        assert(lineage_replace_classify_reconciliation_transition(&facts) == transition(fields[5])); cases += 1u;
    }
    fclose(fp); assert(version == 1); assert(cases == 14u); puts("lineage-replace reconciliation provenance corpus: ok"); return EXIT_SUCCESS;
}
