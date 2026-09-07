#ifndef AUTH_DATA_AUDIT_CHAIN_H
#define AUTH_DATA_AUDIT_CHAIN_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    DATA_AUDIT_EVENT_RELEASE_GRANTED = 1,
    DATA_AUDIT_EVENT_RELEASE_DENIED = 2
} data_audit_event_kind_t;

typedef struct {
    uint8_t release_context_hash[32];
    uint64_t authorization_generation;
    uint64_t revocation_epoch;
    uint64_t policy_epoch;
    data_audit_event_kind_t kind;
} data_audit_event_t;

typedef struct {
    uint8_t head[32];
    uint64_t next_sequence;
} data_audit_state_t;

typedef enum {
    DATA_AUDIT_OK = 0,
    DATA_AUDIT_INVALID_ARGUMENT,
    DATA_AUDIT_INVALID_SEQUENCE,
    DATA_AUDIT_SEQUENCE_EXHAUSTED
} data_audit_result_t;

void data_audit_state_genesis(data_audit_state_t *state);

data_audit_result_t data_audit_entry_digest(
    uint8_t out[32],
    const uint8_t previous_head[32],
    uint64_t sequence,
    const data_audit_event_t *event);

data_audit_result_t data_audit_append(
    data_audit_state_t *state,
    const data_audit_event_t *event,
    uint8_t out[32]);

bool data_audit_verify_transition(
    const uint8_t previous_head[32],
    uint64_t sequence,
    const data_audit_event_t *event,
    const uint8_t expected_head[32]);

#endif
