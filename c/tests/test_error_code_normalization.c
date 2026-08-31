/*
 * test_error_code_normalization.c — cross-language ERROR-code decode parity.
 *
 * Rust maps every unregistered u16 ERROR code to UNSPECIFIED. The C decoder
 * must do the same so callers cannot observe language-dependent semantics for
 * the same wire bytes. Local-only C API errors are not wire allocations.
 */

#include "auth/auth_wire.h"

#include <stdio.h>
#include <string.h>

static int failures = 0;

#define CHECK(expr, msg) do { \
    if (!(expr)) { \
        fprintf(stderr, "FAIL: %s\n", msg); \
        failures++; \
    } \
} while (0)

static void check_code(const uint8_t *payload, size_t payload_len,
                       auth_err_t expected, const char *label)
{
    auth_err_t code = AUTH_OK;
    const char *msg = NULL;
    size_t msg_len = 0;
    auth_err_t err = auth_packet_parse_error(
        payload, payload_len, &code, &msg, &msg_len);

    CHECK(err == AUTH_OK, label);
    CHECK(code == expected, label);
    if (payload_len >= 2) {
        CHECK(msg == (const char *)(payload + 2), label);
        CHECK(msg_len == payload_len - 2, label);
    }
}

int main(void)
{
    const uint8_t known_replay[] = { 0x03, 0x04, 'r' };
    const uint8_t known_unspecified[] = { 0xff, 0x7f };
    const uint8_t unknown_in_registered_range[] = { 0x04, 0x01, 'x' };
    const uint8_t unknown_category[] = { 0x34, 0x12, 'y', 'z' };
    const uint8_t local_only_code[] = { 0x01, 0x00 };
    const uint8_t short_payload[] = { 0x03 };

    check_code(known_replay, sizeof known_replay,
               AUTH_ERR_REPLAY_DETECTED, "known code preserved");
    check_code(known_unspecified, sizeof known_unspecified,
               AUTH_ERR_UNSPECIFIED, "UNSPECIFIED preserved");
    check_code(unknown_in_registered_range, sizeof unknown_in_registered_range,
               AUTH_ERR_UNSPECIFIED, "unknown registered-range code normalized");
    check_code(unknown_category, sizeof unknown_category,
               AUTH_ERR_UNSPECIFIED, "unknown category code normalized");
    check_code(local_only_code, sizeof local_only_code,
               AUTH_ERR_UNSPECIFIED, "local-only code rejected as wire allocation");

    auth_err_t code = AUTH_OK;
    CHECK(auth_packet_parse_error(short_payload, sizeof short_payload,
                                  &code, NULL, NULL) == AUTH_ERR_MALFORMED_PACKET,
          "short ERROR payload rejected");

    if (failures) {
        fprintf(stderr, "FAIL: %d failure(s)\n", failures);
        return 1;
    }
    printf("PASS: error-code normalization\n");
    return 0;
}
