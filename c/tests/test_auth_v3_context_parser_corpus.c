#include "auth/auth_v3_context_parser.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CORPUS_CASES_EXPECTED 13u

static uint8_t hex_nibble(char c) {
    if (c >= '0' && c <= '9') return (uint8_t)(c - '0');
    if (c >= 'a' && c <= 'f') return (uint8_t)(c - 'a' + 10);
    fputs("invalid shared parser corpus hex\n", stderr);
    abort();
}

static size_t decode_hex(const char *hex, uint8_t *out, size_t out_cap) {
    size_t len = strlen(hex);
    size_t i;

    if ((len % 2u) != 0u || out_cap < len / 2u) {
        fputs("invalid shared parser corpus hex length\n", stderr);
        abort();
    }

    for (i = 0u; i < len / 2u; ++i) {
        out[i] = (uint8_t)((hex_nibble(hex[i * 2u]) << 4) |
                           hex_nibble(hex[i * 2u + 1u]));
    }
    return len / 2u;
}

static int expected_error(const char *token) {
    if (strcmp(token, "TRUNCATED") == 0) return AUTH_V3_CONTEXT_PARSE_TRUNCATED;
    if (strcmp(token, "INVALID_MAGIC") == 0) return AUTH_V3_CONTEXT_PARSE_INVALID_MAGIC;
    if (strcmp(token, "UNSUPPORTED_VERSION") == 0) return AUTH_V3_CONTEXT_PARSE_UNSUPPORTED_VERSION;
    if (strcmp(token, "UNKNOWN_KIND") == 0) return AUTH_V3_CONTEXT_PARSE_UNKNOWN_KIND;
    if (strcmp(token, "INVALID_ID") == 0) return AUTH_V3_CONTEXT_PARSE_INVALID_ID;
    if (strcmp(token, "NON_CANONICAL_ORDER") == 0) {
        return AUTH_V3_CONTEXT_PARSE_NON_CANONICAL_ORDER;
    }
    if (strcmp(token, "INVALID_CRITICAL_ID") == 0) {
        return AUTH_V3_CONTEXT_PARSE_INVALID_CRITICAL_ID;
    }
    if (strcmp(token, "NONZERO_FLAGS") == 0) return AUTH_V3_CONTEXT_PARSE_NONZERO_FLAGS;
    if (strcmp(token, "TRAILING_BYTES") == 0) return AUTH_V3_CONTEXT_PARSE_TRAILING_BYTES;

    fprintf(stderr, "unknown shared parser corpus error token: %s\n", token);
    abort();
}

static FILE *open_corpus(void) {
    static const char *paths[] = {
        "../rust/test-vectors/auth-v3/context-parser-negative-v1.txt",
        "rust/test-vectors/auth-v3/context-parser-negative-v1.txt"
    };
    size_t i;

    for (i = 0u; i < sizeof(paths) / sizeof(paths[0]); ++i) {
        FILE *file = fopen(paths[i], "r");
        if (file != NULL) return file;
    }

    fputs("unable to open shared AUTH-v3 parser corpus\n", stderr);
    abort();
}

static void check_case(const char *name, const char *error_token, const char *hex) {
    uint8_t input[256];
    uint8_t digest[32];
    auth_v3_context_kind_t kind = AUTH_V3_CONTEXT_AUTHORIZATION;
    auth_v3_context_entry_t entries[8];
    size_t entry_count = 0u;
    size_t input_len = decode_hex(hex, input, sizeof(input));
    int expected = expected_error(error_token);
    int parse_result = auth_v3_context_parse_bytes(
        input, input_len, &kind, entries, 8u, &entry_count);
    int hash_result = auth_v3_context_hash_bytes(input, input_len, digest);

    if (parse_result != expected || hash_result != expected) {
        fprintf(stderr,
                "shared parser corpus mismatch for %s: expected=%d parse=%d hash=%d\n",
                name, expected, parse_result, hash_result);
        abort();
    }
}

int main(void) {
    FILE *corpus = open_corpus();
    char line[1024];
    size_t case_count = 0u;

    while (fgets(line, sizeof(line), corpus) != NULL) {
        char *first;
        char *second;
        char *name;
        char *error_token;
        char *hex;

        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0' || line[0] == '#') continue;

        first = strchr(line, '|');
        if (first == NULL) {
            fputs("malformed shared parser corpus line\n", stderr);
            abort();
        }
        *first = '\0';

        second = strchr(first + 1, '|');
        if (second == NULL || strchr(second + 1, '|') != NULL) {
            fputs("malformed shared parser corpus line\n", stderr);
            abort();
        }
        *second = '\0';

        name = line;
        error_token = first + 1;
        hex = second + 1;
        check_case(name, error_token, hex);
        case_count += 1u;
    }

    if (ferror(corpus) != 0 || fclose(corpus) != 0) {
        fputs("failed while reading shared parser corpus\n", stderr);
        abort();
    }
    if (case_count != CORPUS_CASES_EXPECTED) {
        fprintf(stderr, "shared parser corpus case-count drift: %zu\n", case_count);
        abort();
    }

    puts("AUTH v3 shared canonical parser negative corpus: ok");
    return 0;
}
