#include "auth/auth_v3_iot_core_authz.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/auth-v3/iot-core-authorization-v1.txt"
#define ATTRIBUTION_VECTOR_PATH "../rust/test-vectors/auth-v3/iot-core-attribution-decisions-v1.txt"

static int read_vector_value(const char *key, char *out, size_t out_capacity) {
    FILE *fp = fopen(VECTOR_PATH, "r");
    char line[1024];
    size_t key_len = strlen(key);
    if (fp == NULL) return -1;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, key, key_len) == 0 && line[key_len] == '=') {
            size_t len = strcspn(line + key_len + 1u, "\r\n");
            if (len + 1u > out_capacity) { fclose(fp); return -1; }
            memcpy(out, line + key_len + 1u, len);
            out[len] = '\0';
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    return -1;
}

static uint8_t hex_nibble(char c) {
    if (c >= '0' && c <= '9') return (uint8_t)(c - '0');
    if (c >= 'a' && c <= 'f') return (uint8_t)(10 + (c - 'a'));
    if (c >= 'A' && c <= 'F') return (uint8_t)(10 + (c - 'A'));
    abort();
}

static void decode_hex_exact(const char *hex, uint8_t *out, size_t out_len) {
    size_t i;
    assert(strlen(hex) == out_len * 2u);
    for (i = 0u; i < out_len; ++i) {
        out[i] = (uint8_t)((hex_nibble(hex[i * 2u]) << 4) | hex_nibble(hex[i * 2u + 1u]));
    }
}

static uint64_t read_u64(const char *key) {
    char value[64];
    char *end = NULL;
    unsigned long long parsed;
    assert(read_vector_value(key, value, sizeof(value)) == 0);
    parsed = strtoull(value, &end, 10);
    assert(end != NULL && *end == '\0');
    return (uint64_t)parsed;
}

static auth_v3_iot_core_authorization_context_v1_t fixture(void) {
    auth_v3_iot_core_authorization_context_v1_t context;
    char value[256];
    memset(&context, 0, sizeof(context));
    assert(read_vector_value("holder_binding", value, sizeof(value)) == 0);
    decode_hex_exact(value, context.holder_binding, sizeof(context.holder_binding));
    assert(read_vector_value("audience_id", value, sizeof(value)) == 0);
    decode_hex_exact(value, context.audience_id, sizeof(context.audience_id));
    context.role_policy_id = read_u64("role_policy_id");
    context.scope_bits = read_u64("scope_bits");
    context.authorization_generation = read_u64("authorization_generation");
    context.policy_epoch = read_u64("policy_epoch");
    context.revocation_epoch = read_u64("revocation_epoch");
    return context;
}

static auth_v3_iot_core_attribution_record_v1_t attribution_fixture(void) {
    auth_v3_iot_core_authorization_context_v1_t context = fixture();
    auth_v3_iot_core_attribution_record_v1_t record;
    memset(&record, 0, sizeof(record));
    memset(record.credential_reference, 0xa1, sizeof(record.credential_reference));
    memset(record.peer_identity, 0xb1, sizeof(record.peer_identity));
    memcpy(record.holder_binding, context.holder_binding, sizeof(record.holder_binding));
    memcpy(record.audience_id, context.audience_id, sizeof(record.audience_id));
    record.role_policy_id = context.role_policy_id;
    record.scope_bits = context.scope_bits;
    record.authorization_generation = context.authorization_generation;
    record.policy_epoch = context.policy_epoch;
    record.revocation_epoch = context.revocation_epoch;
    return record;
}

static void test_shared_vector(void) {
    auth_v3_iot_core_authorization_context_v1_t context = fixture();
    uint8_t encoded[AUTH_V3_IOT_CORE_AUTHZ_CANONICAL_LEN];
    uint8_t expected_encoded[AUTH_V3_IOT_CORE_AUTHZ_CANONICAL_LEN];
    uint8_t hash[32], expected_hash[32];
    char value[512];
    size_t encoded_len = 0u;
    assert(auth_v3_iot_core_authz_encode(&context, encoded, sizeof(encoded), &encoded_len) == AUTH_V3_IOT_CORE_AUTHZ_OK);
    assert(encoded_len == AUTH_V3_IOT_CORE_AUTHZ_CANONICAL_LEN);
    assert(read_vector_value("encoded", value, sizeof(value)) == 0);
    decode_hex_exact(value, expected_encoded, sizeof(expected_encoded));
    assert(memcmp(encoded, expected_encoded, sizeof(encoded)) == 0);
    assert(auth_v3_iot_core_authz_hash(&context, hash) == AUTH_V3_IOT_CORE_AUTHZ_OK);
    assert(read_vector_value("sha256", value, sizeof(value)) == 0);
    decode_hex_exact(value, expected_hash, sizeof(expected_hash));
    assert(memcmp(hash, expected_hash, sizeof(hash)) == 0);
}

static void test_receive_profile_bounds(void) {
    auth_v3_iot_core_authorization_context_v1_t expected = fixture(), decoded;
    uint8_t encoded[AUTH_V3_IOT_CORE_AUTHZ_CANONICAL_LEN + 1u];
    uint8_t hash[32], expected_hash[32];
    char value[512];
    assert(read_vector_value("encoded", value, sizeof(value)) == 0);
    decode_hex_exact(value, encoded, AUTH_V3_IOT_CORE_AUTHZ_CANONICAL_LEN);
    assert(auth_v3_iot_core_authz_decode_bytes(encoded, AUTH_V3_IOT_CORE_AUTHZ_CANONICAL_LEN, &decoded) == AUTH_V3_IOT_CORE_AUTHZ_OK);
    assert(memcmp(decoded.holder_binding, expected.holder_binding, sizeof(decoded.holder_binding)) == 0);
    assert(memcmp(decoded.audience_id, expected.audience_id, sizeof(decoded.audience_id)) == 0);
    assert(decoded.role_policy_id == expected.role_policy_id && decoded.scope_bits == expected.scope_bits);
    assert(decoded.authorization_generation == expected.authorization_generation);
    assert(decoded.policy_epoch == expected.policy_epoch && decoded.revocation_epoch == expected.revocation_epoch);
    assert(auth_v3_iot_core_authz_hash_bytes(encoded, AUTH_V3_IOT_CORE_AUTHZ_CANONICAL_LEN, hash) == AUTH_V3_IOT_CORE_AUTHZ_OK);
    assert(read_vector_value("sha256", value, sizeof(value)) == 0);
    decode_hex_exact(value, expected_hash, sizeof(expected_hash));
    assert(memcmp(hash, expected_hash, sizeof(hash)) == 0);
    assert(auth_v3_iot_core_authz_decode_bytes(encoded, AUTH_V3_IOT_CORE_AUTHZ_CANONICAL_LEN - 1u, &decoded) == AUTH_V3_IOT_CORE_AUTHZ_INVALID_ENCODING_LENGTH);
    encoded[AUTH_V3_IOT_CORE_AUTHZ_CANONICAL_LEN] = 0u;
    assert(auth_v3_iot_core_authz_decode_bytes(encoded, AUTH_V3_IOT_CORE_AUTHZ_CANONICAL_LEN + 1u, &decoded) == AUTH_V3_IOT_CORE_AUTHZ_INVALID_ENCODING_LENGTH);
}

static void test_semantic_rejections(void) {
    auth_v3_iot_core_authorization_context_v1_t base = fixture(), c;
    c = base; memset(c.holder_binding, 0, sizeof(c.holder_binding)); assert(auth_v3_iot_core_authz_validate(&c) == AUTH_V3_IOT_CORE_AUTHZ_INVALID_HOLDER);
    c = base; memset(c.audience_id, 0, sizeof(c.audience_id)); assert(auth_v3_iot_core_authz_validate(&c) == AUTH_V3_IOT_CORE_AUTHZ_INVALID_AUDIENCE);
    c = base; c.role_policy_id = 0u; assert(auth_v3_iot_core_authz_validate(&c) == AUTH_V3_IOT_CORE_AUTHZ_INVALID_ROLE_POLICY);
    c = base; c.scope_bits = 0u; assert(auth_v3_iot_core_authz_validate(&c) == AUTH_V3_IOT_CORE_AUTHZ_INVALID_SCOPE);
    c = base; c.authorization_generation = 0u; assert(auth_v3_iot_core_authz_validate(&c) == AUTH_V3_IOT_CORE_AUTHZ_INVALID_GENERATION);
    c = base; c.policy_epoch = 0u; assert(auth_v3_iot_core_authz_validate(&c) == AUTH_V3_IOT_CORE_AUTHZ_INVALID_POLICY_EPOCH);
    c = base; c.revocation_epoch = 0u; assert(auth_v3_iot_core_authz_validate(&c) == AUTH_V3_IOT_CORE_AUTHZ_INVALID_REVOCATION_EPOCH);
}

static void test_attribution_resolver(void) {
    auth_v3_iot_core_authorization_context_v1_t context = fixture();
    auth_v3_iot_core_attribution_record_v1_t records[2];
    const auth_v3_iot_core_attribution_record_v1_t *resolved = NULL;
    uint8_t missing[32], wrong_peer[32];

    records[0] = attribution_fixture();
    memset(missing, 0xcc, sizeof(missing));
    memset(wrong_peer, 0xb2, sizeof(wrong_peer));

    assert(auth_v3_iot_core_attribution_resolve(records, 1u, records[0].credential_reference,
                                                 records[0].peer_identity, &context, &resolved) == AUTH_V3_IOT_CORE_AUTHZ_OK);
    assert(resolved == &records[0]);
    assert(auth_v3_iot_core_attribution_resolve(records, 1u, missing,
                                                 records[0].peer_identity, &context, &resolved) == AUTH_V3_IOT_CORE_ATTRIBUTION_MISSING_REFERENCE);
    records[1] = records[0];
    memset(records[1].peer_identity, 0xb2, sizeof(records[1].peer_identity));
    assert(auth_v3_iot_core_attribution_resolve(records, 2u, records[0].credential_reference,
                                                 records[0].peer_identity, &context, &resolved) == AUTH_V3_IOT_CORE_ATTRIBUTION_AMBIGUOUS_REFERENCE);
    assert(auth_v3_iot_core_attribution_resolve(records, 1u, records[0].credential_reference,
                                                 wrong_peer, &context, &resolved) == AUTH_V3_IOT_CORE_ATTRIBUTION_IDENTITY_MISMATCH);
    records[0].authorization_generation += 1u;
    assert(auth_v3_iot_core_attribution_resolve(records, 1u, records[0].credential_reference,
                                                 records[0].peer_identity, &context, &resolved) == AUTH_V3_IOT_CORE_ATTRIBUTION_AUTHORIZATION_MISMATCH);
    records[0] = attribution_fixture();
    records[1] = records[0];
    memset(records[1].credential_reference, 0xa2, sizeof(records[1].credential_reference));
    memset(records[1].peer_identity, 0xb2, sizeof(records[1].peer_identity));
    assert(auth_v3_iot_core_attribution_resolve(records, 2u, records[1].credential_reference,
                                                 records[1].peer_identity, &context, &resolved) == AUTH_V3_IOT_CORE_AUTHZ_OK);
    assert(resolved == &records[1]);
}

static int expected_decision(const char *name) {
    if (strcmp(name, "OK") == 0) return AUTH_V3_IOT_CORE_AUTHZ_OK;
    if (strcmp(name, "MISSING_REFERENCE") == 0) return AUTH_V3_IOT_CORE_ATTRIBUTION_MISSING_REFERENCE;
    if (strcmp(name, "AMBIGUOUS_REFERENCE") == 0) return AUTH_V3_IOT_CORE_ATTRIBUTION_AMBIGUOUS_REFERENCE;
    if (strcmp(name, "IDENTITY_MISMATCH") == 0) return AUTH_V3_IOT_CORE_ATTRIBUTION_IDENTITY_MISMATCH;
    if (strcmp(name, "AUTHORIZATION_MISMATCH") == 0) return AUTH_V3_IOT_CORE_ATTRIBUTION_AUTHORIZATION_MISMATCH;
    assert(0 && "unknown expected decision");
    return -1;
}

static void run_attribution_corpus_case(const char *name, const char *mutation, const char *expected) {
    auth_v3_iot_core_authorization_context_v1_t context = fixture();
    auth_v3_iot_core_attribution_record_v1_t records[2];
    const auth_v3_iot_core_attribution_record_v1_t *resolved = NULL;
    uint8_t credential_reference[32], expected_peer_identity[32];
    size_t record_count = 1u;
    int actual;

    records[0] = attribution_fixture();
    memcpy(credential_reference, records[0].credential_reference, sizeof(credential_reference));
    memcpy(expected_peer_identity, records[0].peer_identity, sizeof(expected_peer_identity));

    if (strcmp(mutation, "none") == 0) {
        /* no mutation */
    } else if (strcmp(mutation, "empty_records") == 0) {
        record_count = 0u;
    } else if (strcmp(mutation, "duplicate_reference_peer_b2") == 0) {
        records[1] = records[0];
        memset(records[1].peer_identity, 0xb2, sizeof(records[1].peer_identity));
        record_count = 2u;
    } else if (strcmp(mutation, "expected_peer_b2") == 0) {
        memset(expected_peer_identity, 0xb2, sizeof(expected_peer_identity));
    } else if (strcmp(mutation, "record_generation_minus_1") == 0) {
        records[0].authorization_generation -= 1u;
    } else if (strcmp(mutation, "record_role_plus_1") == 0) {
        records[0].role_policy_id += 1u;
    } else if (strcmp(mutation, "record_audience_flip_0") == 0) {
        records[0].audience_id[0] ^= 1u;
    } else if (strcmp(mutation, "second_ref_peer_b2_query_first_ref_peer_b2") == 0) {
        records[1] = records[0];
        memset(records[1].credential_reference, 0xa2, sizeof(records[1].credential_reference));
        memset(records[1].peer_identity, 0xb2, sizeof(records[1].peer_identity));
        memset(expected_peer_identity, 0xb2, sizeof(expected_peer_identity));
        record_count = 2u;
    } else if (strcmp(mutation, "second_ref_peer_b2_query_second") == 0) {
        records[1] = records[0];
        memset(records[1].credential_reference, 0xa2, sizeof(records[1].credential_reference));
        memset(records[1].peer_identity, 0xb2, sizeof(records[1].peer_identity));
        memset(credential_reference, 0xa2, sizeof(credential_reference));
        memset(expected_peer_identity, 0xb2, sizeof(expected_peer_identity));
        record_count = 2u;
    } else {
        fprintf(stderr, "unknown attribution corpus mutation for %s: %s\n", name, mutation);
        assert(0);
    }

    actual = auth_v3_iot_core_attribution_resolve(records, record_count, credential_reference,
                                                   expected_peer_identity, &context, &resolved);
    assert(actual == expected_decision(expected));
}

static void test_shared_attribution_decision_corpus(void) {
    FILE *fp = fopen(ATTRIBUTION_VECTOR_PATH, "r");
    char line[512];
    unsigned case_count = 0u;
    int saw_version = 0;
    assert(fp != NULL);

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *name, *mutation, *expected, *extra;
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
        run_attribution_corpus_case(name, mutation, expected);
        case_count += 1u;
    }
    fclose(fp);
    assert(saw_version == 1);
    assert(case_count > 0u);
}

int main(void) {
    test_shared_vector();
    test_receive_profile_bounds();
    test_semantic_rejections();
    test_attribution_resolver();
    test_shared_attribution_decision_corpus();
    puts("AUTH v3 iot-core authorization context: ok");
    return 0;
}
