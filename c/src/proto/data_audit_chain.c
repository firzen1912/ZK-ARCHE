#include "auth/data_audit_chain.h"

#include <limits.h>
#include <sodium.h>
#include <string.h>

static const uint8_t DOMAIN[] = "ZKARCHE-DATA-AUDIT-v1";

static void update_u64(crypto_hash_sha256_state *state, uint64_t value) {
    uint8_t be[8];
    size_t i;
    for (i = 0u; i < sizeof be; ++i) {
        be[sizeof be - 1u - i] = (uint8_t)(value & 0xffu);
        value >>= 8;
    }
    (void)crypto_hash_sha256_update(state, be, sizeof be);
}

void data_audit_state_genesis(data_audit_state_t *state) {
    if (state == NULL) {
        return;
    }
    memset(state->head, 0, sizeof state->head);
    state->next_sequence = 1u;
}

data_audit_result_t data_audit_entry_digest(
    uint8_t out[32],
    const uint8_t previous_head[32],
    uint64_t sequence,
    const data_audit_event_t *event)
{
    crypto_hash_sha256_state state;
    uint8_t kind;

    if (out == NULL || previous_head == NULL || event == NULL) {
        return DATA_AUDIT_INVALID_ARGUMENT;
    }
    if (sequence == 0u) {
        return DATA_AUDIT_INVALID_SEQUENCE;
    }
    if (event->kind != DATA_AUDIT_EVENT_RELEASE_GRANTED
        && event->kind != DATA_AUDIT_EVENT_RELEASE_DENIED) {
        return DATA_AUDIT_INVALID_ARGUMENT;
    }

    kind = (uint8_t)event->kind;
    (void)crypto_hash_sha256_init(&state);
    (void)crypto_hash_sha256_update(&state, DOMAIN, sizeof DOMAIN - 1u);
    (void)crypto_hash_sha256_update(&state, previous_head, 32u);
    update_u64(&state, sequence);
    (void)crypto_hash_sha256_update(&state, &kind, 1u);
    (void)crypto_hash_sha256_update(&state, event->release_context_hash, 32u);
    update_u64(&state, event->authorization_generation);
    update_u64(&state, event->revocation_epoch);
    update_u64(&state, event->policy_epoch);
    (void)crypto_hash_sha256_final(&state, out);
    return DATA_AUDIT_OK;
}

data_audit_result_t data_audit_append(
    data_audit_state_t *state,
    const data_audit_event_t *event,
    uint8_t out[32])
{
    data_audit_result_t rc;
    uint8_t digest[32];

    if (state == NULL || event == NULL || out == NULL) {
        return DATA_AUDIT_INVALID_ARGUMENT;
    }
    if (state->next_sequence == UINT64_MAX) {
        return DATA_AUDIT_SEQUENCE_EXHAUSTED;
    }

    rc = data_audit_entry_digest(digest, state->head, state->next_sequence, event);
    if (rc != DATA_AUDIT_OK) {
        return rc;
    }
    memcpy(state->head, digest, sizeof state->head);
    memcpy(out, digest, sizeof digest);
    state->next_sequence += 1u;
    return DATA_AUDIT_OK;
}

bool data_audit_verify_transition(
    const uint8_t previous_head[32],
    uint64_t sequence,
    const data_audit_event_t *event,
    const uint8_t expected_head[32])
{
    uint8_t actual[32];
    if (expected_head == NULL) {
        return false;
    }
    if (data_audit_entry_digest(actual, previous_head, sequence, event) != DATA_AUDIT_OK) {
        return false;
    }
    return sodium_memcmp(actual, expected_head, sizeof actual) == 0;
}
