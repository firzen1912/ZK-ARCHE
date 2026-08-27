#include <sodium.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("  FAIL: %s\n", msg); failures++; } \
} while (0)

static FILE *open_contract(void)
{
    static const char *paths[] = {
        "../rust/test-vectors/profiles/iot-core-v1.profile",
        "rust/test-vectors/profiles/iot-core-v1.profile"
    };

    for (size_t i = 0; i < sizeof paths / sizeof paths[0]; ++i) {
        FILE *f = fopen(paths[i], "rb");
        if (f != NULL) return f;
    }
    return NULL;
}

static size_t read_contract(char *buf, size_t cap)
{
    FILE *f = open_contract();
    CHECK(f != NULL, "open shared iot-core profile contract");
    if (f == NULL) return 0u;

    size_t n = fread(buf, 1u, cap - 1u, f);
    CHECK(!ferror(f), "read shared iot-core profile contract");
    CHECK(!feof(f) || n < cap - 1u, "profile contract fits test buffer");
    fclose(f);
    buf[n] = '\0';
    return n;
}

static int field_value(const char *buf, const char *key, char *out, size_t out_cap)
{
    size_t key_len = strlen(key);
    const char *p = buf;
    int matches = 0;

    while (*p != '\0') {
        const char *line_end = strchr(p, '\n');
        if (line_end == NULL) return 0;
        size_t line_len = (size_t)(line_end - p);
        if (line_len > key_len && p[key_len] == '=' && memcmp(p, key, key_len) == 0) {
            size_t value_len = line_len - key_len - 1u;
            CHECK(value_len + 1u <= out_cap, "profile value fits output buffer");
            if (value_len + 1u > out_cap) return 0;
            memcpy(out, p + key_len + 1u, value_len);
            out[value_len] = '\0';
            matches++;
        }
        p = line_end + 1;
    }

    CHECK(matches == 1, "profile key appears exactly once");
    return matches == 1;
}

static uint64_t parse_hex_u64(const char *value)
{
    CHECK(value[0] == '0' && value[1] == 'x', "canonical hex prefix");
    char *end = NULL;
    unsigned long long parsed = strtoull(value + 2, &end, 16);
    CHECK(end != NULL && *end == '\0', "canonical u64 hex parse");
    return (uint64_t)parsed;
}

static void expect_field(const char *buf, const char *key, const char *expected)
{
    char value[128];
    if (!field_value(buf, key, value, sizeof value)) return;
    CHECK(strcmp(value, expected) == 0, key);
}

static void test_profile_contract(void)
{
    printf("== shared iot-core profile contract ==\n");
    char buf[4096];
    size_t n = read_contract(buf, sizeof buf);
    if (n == 0u) return;

    const char *marker = strstr(buf, "contract_sha256=");
    CHECK(marker != NULL, "profile fingerprint marker present");
    if (marker == NULL) return;

    unsigned char digest[crypto_hash_sha256_BYTES];
    char digest_hex[crypto_hash_sha256_BYTES * 2u + 1u];
    crypto_hash_sha256(digest, (const unsigned char *)buf, (unsigned long long)(marker - buf));
    sodium_bin2hex(digest_hex, sizeof digest_hex, digest, sizeof digest);

    char recorded[128];
    CHECK(field_value(buf, "contract_sha256", recorded, sizeof recorded),
          "read profile fingerprint");
    CHECK(strcmp(digest_hex, recorded) == 0, "profile fingerprint matches canonical body");
    CHECK(strcmp(digest_hex,
                 "31b53234616189ce470c8c7f2d3d446432bb20953a2f4e5a191fd356a1f54ad4") == 0,
          "profile fingerprint remains stable");

    expect_field(buf, "format", "ZKPROFILE/1");
    expect_field(buf, "profile_id", "0x0001");
    expect_field(buf, "name", "iot-core");
    expect_field(buf, "definition_revision", "1");
    expect_field(buf, "status", "draft");
    expect_field(buf, "selectable", "0");
    expect_field(buf, "protocol_version", "0x03");
    expect_field(buf, "suite_id", "0x0001");
    expect_field(buf, "authorization_schema", "iot-core-authz-v1");
    expect_field(buf, "authorization_context_bytes", "148");
    expect_field(buf, "critical_extensions_policy", "none");
    expect_field(buf, "channel_binding_policy", "none");
    expect_field(buf, "max_datagram_bytes", "2048");

    char required_text[128];
    char allowed_text[128];
    char forbidden_text[128];
    CHECK(field_value(buf, "required_selected_capabilities", required_text, sizeof required_text),
          "read required capabilities");
    CHECK(field_value(buf, "allowed_selected_capabilities", allowed_text, sizeof allowed_text),
          "read allowed capabilities");
    CHECK(field_value(buf, "forbidden_selected_capabilities", forbidden_text, sizeof forbidden_text),
          "read forbidden capabilities");

    uint64_t required = parse_hex_u64(required_text);
    uint64_t allowed = parse_hex_u64(allowed_text);
    uint64_t forbidden = parse_hex_u64(forbidden_text);
    CHECK(required == UINT64_C(0x6), "required role-proof capabilities");
    CHECK(allowed == UINT64_C(0x6), "allowed selected capabilities");
    CHECK((required & ~allowed) == 0u, "required capabilities are allowed");
    CHECK((allowed & forbidden) == 0u, "allowed and forbidden masks do not overlap");
    CHECK((allowed | forbidden) == UINT64_MAX, "capability masks cover selected namespace");

    expect_field(buf, "replay_policy", "unresolved");
    expect_field(buf, "replay_min_entries", "unresolved");
    expect_field(buf, "replay_epoch_rule", "unresolved");
    expect_field(buf, "restart_replay_rule", "unresolved");
    expect_field(buf, "resource_evidence", "required-before-stable");
    expect_field(buf, "prescriptive_change_rule", "new-profile-id");
    expect_field(buf, "deprecated_selectable", "0");
    expect_field(buf, "stable_semantics_mutable", "0");
}

int main(void)
{
    if (sodium_init() < 0) return 1;
    test_profile_contract();
    printf("\n%s: %d failure(s)\n", failures ? "FAIL" : "PASS", failures);
    return failures ? 1 : 0;
}
