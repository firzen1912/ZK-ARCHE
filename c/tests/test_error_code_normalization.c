/*
 * Shared Rust/C ERROR-code normalization corpus consumer.
 *
 * The canonical corpus lives under rust/test-vectors/ and enumerates every
 * registered wire ERROR code plus representative unassigned/local-only values.
 */

#include "auth/auth_wire.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VECTOR_PATH "../rust/test-vectors/wire/error-code-normalization-v1.txt"

static uint16_t parse_hex_u16(const char *value)
{
    char *end = NULL;
    unsigned long parsed = strtoul(value, &end, 16);
    assert(end != value && *end == '\0' && parsed <= 0xfffful);
    return (uint16_t)parsed;
}

int main(void)
{
    FILE *fp = fopen(VECTOR_PATH, "r");
    char line[256];
    unsigned cases = 0u;
    int version = 0;

    assert(fp != NULL);
    while (fgets(line, sizeof(line), fp) != NULL) {
        char *fields[3] = {0};
        char *part;
        unsigned i = 0u;
        uint16_t received;
        uint16_t expected;
        uint8_t payload[3];
        auth_err_t code = AUTH_OK;
        const char *message = NULL;
        size_t message_len = 0u;

        line[strcspn(line, "\r\n")] = '\0';
        if (strcmp(line, "version=1") == 0) {
            version = 1;
            continue;
        }
        if (strncmp(line, "case=", 5u) != 0) {
            continue;
        }

        part = strtok(line + 5u, "|");
        while (part != NULL && i < 3u) {
            fields[i++] = part;
            part = strtok(NULL, "|");
        }
        assert(i == 3u && part == NULL);

        received = parse_hex_u16(fields[1]);
        expected = parse_hex_u16(fields[2]);
        payload[0] = (uint8_t)(received & 0xffu);
        payload[1] = (uint8_t)(received >> 8);
        payload[2] = (uint8_t)'x';

        assert(auth_packet_parse_error(payload, sizeof(payload), &code,
                                       &message, &message_len) == AUTH_OK);
        assert((uint16_t)code == expected);
        assert(message == (const char *)(payload + 2));
        assert(message_len == 1u && message[0] == 'x');
        cases += 1u;
    }
    fclose(fp);

    assert(version == 1 && cases == 35u);

    {
        const uint8_t short_payload[] = {0x03};
        auth_err_t code = AUTH_OK;
        assert(auth_packet_parse_error(short_payload, sizeof(short_payload),
                                       &code, NULL, NULL) == AUTH_ERR_MALFORMED_PACKET);
    }

    puts("error-code normalization corpus: ok");
    return EXIT_SUCCESS;
}
