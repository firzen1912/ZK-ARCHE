#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "auth/lineage_replace_session_binding.h"

static void fill(uint8_t *value, size_t len, uint8_t byte) { memset(value, (int)byte, len); }

int main(void) {
    auth_v3_context_t context = {0};
    lineage_replace_session_binding_expectation_t expected = {0};
    context.protocol_version = 3u; context.suite_id = 1u; context.profile_id = 1u;
    fill(context.session_id, AUTH_SESSION_ID_LEN, 1u);
    fill(context.authz_context_hash, AUTH_HASH_LEN, 2u);
    fill(context.channel_binding_hash, AUTH_HASH_LEN, 3u);
    expected.auth_completion_verified = true; expected.expected_protocol_version = 3u;
    expected.expected_suite_id = 1u; expected.expected_profile_id = 1u;
    fill(expected.expected_session_id, AUTH_SESSION_ID_LEN, 1u);
    fill(expected.expected_authz_context_hash, AUTH_HASH_LEN, 2u);
    fill(expected.expected_channel_binding_hash, AUTH_HASH_LEN, 3u);
    assert(lineage_replace_classify_session_binding(&context, &expected) == LINEAGE_REPLACE_SESSION_BOUND);
    expected.auth_completion_verified = false;
    assert(lineage_replace_classify_session_binding(&context, &expected) == LINEAGE_REPLACE_SESSION_REJECT_COMPLETION);
    expected.auth_completion_verified = true;
    context.protocol_version = 2u;
    assert(lineage_replace_classify_session_binding(&context, &expected) == LINEAGE_REPLACE_SESSION_REJECT_VERSION);
    context.protocol_version = 3u; context.suite_id = 2u;
    assert(lineage_replace_classify_session_binding(&context, &expected) == LINEAGE_REPLACE_SESSION_REJECT_SUITE);
    context.suite_id = 1u; context.profile_id = 2u;
    assert(lineage_replace_classify_session_binding(&context, &expected) == LINEAGE_REPLACE_SESSION_REJECT_PROFILE);
    context.profile_id = 1u; context.session_id[0] = 9u;
    assert(lineage_replace_classify_session_binding(&context, &expected) == LINEAGE_REPLACE_SESSION_REJECT_SESSION_ID);
    context.session_id[0] = 1u; context.authz_context_hash[0] = 9u;
    assert(lineage_replace_classify_session_binding(&context, &expected) == LINEAGE_REPLACE_SESSION_REJECT_AUTHZ_CONTEXT);
    context.authz_context_hash[0] = 2u; context.channel_binding_hash[0] = 9u;
    assert(lineage_replace_classify_session_binding(&context, &expected) == LINEAGE_REPLACE_SESSION_REJECT_CHANNEL_BINDING);
    assert(lineage_replace_classify_session_binding(NULL, &expected) == LINEAGE_REPLACE_SESSION_REJECT_COMPLETION);
    puts("lineage-replace session-binding corpus: ok");
    return 0;
}
