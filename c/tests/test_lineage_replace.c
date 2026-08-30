#include "auth/lineage_replace.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-decisions-v1.txt"
#define PLAN_VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-plans-v1.txt"
#define STATE_VECTOR_PATH "../rust/test-vectors/replay/lineage-replace-states-v1.txt"

static lineage_replace_facts_t fixture(void) {
    lineage_replace_facts_t facts = {
        LINEAGE_REPLACE_TRIGGER_LIFECYCLE,
        true, true, true, true, true, true, true, true, true, true
    };
    return facts;
}

static lineage_replace_decision_t expected_decision(const char *name) {
    if (strcmp(name, "ACCEPT_SUCCESSOR") == 0) return LINEAGE_REPLACE_ACCEPT_SUCCESSOR;
    if (strcmp(name, "REJECT_AUTHORITY") == 0) return LINEAGE_REPLACE_REJECT_AUTHORITY;
    if (strcmp(name, "REJECT_PREDECESSOR") == 0) return LINEAGE_REPLACE_REJECT_PREDECESSOR;
    if (strcmp(name, "REJECT_SUCCESSOR") == 0) return LINEAGE_REPLACE_REJECT_SUCCESSOR;
    if (strcmp(name, "REJECT_CONTEXT") == 0) return LINEAGE_REPLACE_REJECT_CONTEXT;
    if (strcmp(name, "REJECT_FRESHNESS") == 0) return LINEAGE_REPLACE_REJECT_FRESHNESS;
    if (strcmp(name, "REJECT_REPLAY") == 0) return LINEAGE_REPLACE_REJECT_REPLAY;
    if (strcmp(name, "REJECT_CONCURRENT") == 0) return LINEAGE_REPLACE_REJECT_CONCURRENT;
    if (strcmp(name, "REJECT_ROLLBACK") == 0) return LINEAGE_REPLACE_REJECT_ROLLBACK;
    if (strcmp(name, "REJECT_STORAGE") == 0) return LINEAGE_REPLACE_REJECT_STORAGE;
    assert(0 && "unknown lineage-replace decision");
    return LINEAGE_REPLACE_REJECT_STORAGE;
}

static lineage_replace_state_t expected_state(const char *name) {
    if (strcmp(name, "ACTIVE_PREDECESSOR") == 0)
        return LINEAGE_REPLACE_STATE_ACTIVE_PREDECESSOR;
    if (strcmp(name, "REPLACEMENT_PENDING") == 0)
        return LINEAGE_REPLACE_STATE_REPLACEMENT_PENDING;
    if (strcmp(name, "ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED") == 0)
        return LINEAGE_REPLACE_STATE_ACTIVE_SUCCESSOR_PREDECESSOR_RETIRED;
    if (strcmp(name, "CONTINUITY_BROKEN") == 0)
        return LINEAGE_REPLACE_STATE_CONTINUITY_BROKEN;
    assert(0 && "unknown lineage-replace state");
    return LINEAGE_REPLACE_STATE_CONTINUITY_BROKEN;
}

static lineage_replace_event_t expected_event(const char *name) {
    if (strcmp(name, "BEGIN") == 0) return LINEAGE_REPLACE_EVENT_BEGIN;
    if (strcmp(name, "COMMIT") == 0) return LINEAGE_REPLACE_EVENT_COMMIT;
    if (strcmp(name, "INTERRUPT") == 0) return LINEAGE_REPLACE_EVENT_INTERRUPT;
    assert(0 && "unknown lineage-replace event");
    return LINEAGE_REPLACE_EVENT_INTERRUPT;
}

static const lineage_replace_plan_t *plan_fixture(const char *marker, lineage_replace_plan_t *plan) {
    if (strcmp(marker, "none") == 0) return NULL;
    assert(lineage_replace_plan(LINEAGE_REPLACE_ACCEPT_SUCCESSOR, plan));
    if (strcmp(marker, "full") == 0) return plan;
    if (strcmp(marker, "partial") == 0) {
        plan->invalidate_replay_state = false;
        return plan;
    }
    assert(0 && "unknown lineage-replace plan marker");
    return NULL;
}

static void apply_mutation(lineage_replace_facts_t *facts, const char *mutation) {
    if (strcmp(mutation, "none") == 0) return;
    if (strcmp(mutation, "unauthorized_authority") == 0) facts->authority_valid = false;
    else if (strcmp(mutation, "restart_trigger") == 0) facts->trigger = LINEAGE_REPLACE_TRIGGER_RESTART;
    else if (strcmp(mutation, "transport_change_trigger") == 0) facts->trigger = LINEAGE_REPLACE_TRIGGER_TRANSPORT_CHANGE;
    else if (strcmp(mutation, "auth_trigger") == 0) facts->trigger = LINEAGE_REPLACE_TRIGGER_AUTH;
    else if (strcmp(mutation, "replayed_transition") == 0 || strcmp(mutation, "predecessor_domain_not_retired") == 0) facts->replay_free = false;
    else if (strcmp(mutation, "storage_partial") == 0 || strcmp(mutation, "storage_ambiguous") == 0) facts->storage_safe = false;
    else if (strcmp(mutation, "rollback_suspected") == 0) facts->rollback_clear = false;
    else if (strcmp(mutation, "stale_freshness") == 0 || strcmp(mutation, "stale_revocation") == 0) facts->freshness_valid = false;
    else if (strcmp(mutation, "dependent_state_not_revalidated") == 0) facts->dependent_state_safe = false;
    else if (strcmp(mutation, "competing_successor") == 0) facts->concurrent_free = false;
    else if (strcmp(mutation, "retired_predecessor") == 0 || strcmp(mutation, "wrong_predecessor_generation") == 0) facts->predecessor_valid = false;
    else if (strcmp(mutation, "successor_binding_mismatch") == 0) facts->successor_valid = false;
    else if (strcmp(mutation, "downgrade_context") == 0 || strcmp(mutation, "wrong_domain_context") == 0) facts->context_valid = false;
    else assert(0 && "unknown lineage-replace mutation");
}

static void test_decision_corpus(void) {
    FILE *fp = fopen(VECTOR_PATH, "r");
    char line[256];
    unsigned case_count = 0u;
    int saw_version = 0;
    assert(fp != NULL);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *name, *mutation, *expected, *extra;
        lineage_replace_facts_t facts;
        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=1") == 0) {
            saw_version = 1;
            continue;
        }
        if (strncmp(line, "case=", 5u) != 0) continue;
        name = strtok(line + 5u, "|");
        mutation = strtok(NULL, "|");
        expected = strtok(NULL, "|");
        extra = strtok(NULL, "|");
        assert(name != NULL && mutation != NULL && expected != NULL && extra == NULL);

        facts = fixture();
        apply_mutation(&facts, mutation);
        assert(lineage_replace_evaluate(&facts) == expected_decision(expected));
        case_count += 1u;
    }

    fclose(fp);
    assert(saw_version == 1);
    assert(case_count == 20u);
}

static void test_plan_corpus(void) {
    FILE *fp = fopen(PLAN_VECTOR_PATH, "r");
    char line[256];
    unsigned case_count = 0u;
    int saw_version = 0;
    assert(fp != NULL);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *decision_name, *should_plan, *extra;
        lineage_replace_plan_t plan;
        bool planned;
        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=1") == 0) {
            saw_version = 1;
            continue;
        }
        if (strncmp(line, "case=", 5u) != 0) continue;
        decision_name = strtok(line + 5u, "|");
        should_plan = strtok(NULL, "|");
        extra = strtok(NULL, "|");
        assert(decision_name != NULL && should_plan != NULL && extra == NULL);

        planned = lineage_replace_plan(expected_decision(decision_name), &plan);
        if (strcmp(should_plan, "1") == 0) {
            assert(planned);
            assert(plan.retire_predecessor);
            assert(plan.activate_successor);
            assert(plan.invalidate_session_keys);
            assert(plan.invalidate_resumption);
            assert(plan.invalidate_authorization_cache);
            assert(plan.invalidate_attribution_cache);
            assert(plan.invalidate_channel_binding);
            assert(plan.invalidate_replay_state);
        } else {
            assert(strcmp(should_plan, "0") == 0);
            assert(!planned);
            assert(!plan.retire_predecessor);
            assert(!plan.activate_successor);
            assert(!plan.invalidate_session_keys);
            assert(!plan.invalidate_resumption);
            assert(!plan.invalidate_authorization_cache);
            assert(!plan.invalidate_attribution_cache);
            assert(!plan.invalidate_channel_binding);
            assert(!plan.invalidate_replay_state);
        }
        case_count += 1u;
    }

    fclose(fp);
    assert(saw_version == 1);
    assert(case_count == 10u);
    assert(!lineage_replace_plan(LINEAGE_REPLACE_ACCEPT_SUCCESSOR, NULL));
}

static void test_state_corpus(void) {
    FILE *fp = fopen(STATE_VECTOR_PATH, "r");
    char line[256];
    unsigned case_count = 0u;
    int saw_version = 0;
    assert(fp != NULL);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *name, *initial_name, *event_name, *plan_marker, *should_advance, *next_name, *extra;
        lineage_replace_plan_t plan;
        lineage_replace_state_t state;
        const lineage_replace_plan_t *plan_ptr;
        bool advanced;
        bool expected_advanced;

        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=1") == 0) {
            saw_version = 1;
            continue;
        }
        if (strncmp(line, "case=", 5u) != 0) continue;
        name = strtok(line + 5u, "|");
        initial_name = strtok(NULL, "|");
        event_name = strtok(NULL, "|");
        plan_marker = strtok(NULL, "|");
        should_advance = strtok(NULL, "|");
        next_name = strtok(NULL, "|");
        extra = strtok(NULL, "|");
        assert(name != NULL && initial_name != NULL && event_name != NULL && plan_marker != NULL);
        assert(should_advance != NULL && next_name != NULL && extra == NULL);

        state = expected_state(initial_name);
        plan_ptr = plan_fixture(plan_marker, &plan);
        advanced = lineage_replace_advance(&state, expected_event(event_name), plan_ptr);
        assert(strcmp(should_advance, "0") == 0 || strcmp(should_advance, "1") == 0);
        expected_advanced = strcmp(should_advance, "1") == 0;
        assert(advanced == expected_advanced);
        assert(state == expected_state(next_name));
        case_count += 1u;
    }

    fclose(fp);
    assert(saw_version == 1);
    assert(case_count == 14u);

    assert(!lineage_replace_advance(NULL, LINEAGE_REPLACE_EVENT_BEGIN, NULL));
}

int main(void) {
    test_decision_corpus();
    test_plan_corpus();
    test_state_corpus();
    puts("lineage-replace shared corpora: ok");
    return 0;
}
