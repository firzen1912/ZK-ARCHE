#include "auth/data_audit_chain.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    static const uint8_t expected[32] = {
        0x9d,0xf3,0x6e,0x00,0xb5,0x56,0x84,0xfc,
        0xd1,0xaf,0x82,0x97,0xcb,0x83,0x2f,0x61,
        0x6b,0x62,0xcd,0xe5,0x18,0x1c,0x0c,0xe9,
        0xf2,0xcd,0x8e,0x99,0x7b,0xe5,0x80,0x98
    };
    data_audit_state_t state;
    data_audit_event_t event;
    uint8_t out[32];
    uint8_t changed_prev[32] = {0};
    size_t i;

    memset(&event, 0, sizeof event);
    for (i = 0u; i < sizeof event.release_context_hash; ++i) {
        event.release_context_hash[i] = (uint8_t)i;
    }
    event.authorization_generation = 7u;
    event.revocation_epoch = 11u;
    event.policy_epoch = 13u;
    event.kind = DATA_AUDIT_EVENT_RELEASE_GRANTED;

    data_audit_state_genesis(&state);
    assert(data_audit_append(&state, &event, out) == DATA_AUDIT_OK);
    assert(memcmp(out, expected, sizeof expected) == 0);
    assert(memcmp(state.head, expected, sizeof expected) == 0);
    assert(state.next_sequence == 2u);
    assert(data_audit_verify_transition((const uint8_t[32]){0}, 1u, &event, expected));

    changed_prev[0] = 1u;
    assert(!data_audit_verify_transition(changed_prev, 1u, &event, expected));
    assert(!data_audit_verify_transition((const uint8_t[32]){0}, 2u, &event, expected));

    event.release_context_hash[0] ^= 1u;
    assert(!data_audit_verify_transition((const uint8_t[32]){0}, 1u, &event, expected));
    event.release_context_hash[0] ^= 1u;

    event.authorization_generation = 8u;
    assert(!data_audit_verify_transition((const uint8_t[32]){0}, 1u, &event, expected));
    event.authorization_generation = 7u;

    event.revocation_epoch = 12u;
    assert(!data_audit_verify_transition((const uint8_t[32]){0}, 1u, &event, expected));
    event.revocation_epoch = 11u;

    event.policy_epoch = 14u;
    assert(!data_audit_verify_transition((const uint8_t[32]){0}, 1u, &event, expected));
    event.policy_epoch = 13u;

    event.kind = DATA_AUDIT_EVENT_RELEASE_DENIED;
    assert(!data_audit_verify_transition((const uint8_t[32]){0}, 1u, &event, expected));
    event.kind = DATA_AUDIT_EVENT_RELEASE_GRANTED;

    assert(data_audit_entry_digest(out, (const uint8_t[32]){0}, 0u, &event) == DATA_AUDIT_INVALID_SEQUENCE);
    state.next_sequence = UINT64_MAX;
    assert(data_audit_append(&state, &event, out) == DATA_AUDIT_SEQUENCE_EXHAUSTED);

    puts("data audit chain: ok");
    return 0;
}
