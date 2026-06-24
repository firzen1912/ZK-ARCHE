#include <stddef.h>
#include <stdint.h>
#include "auth/auth_wire.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auth_header_t hdr;
    const uint8_t *payload = NULL;
    size_t payload_len = 0;
    (void)auth_header_decode(&hdr, data, size, &payload, &payload_len);

    size_t off = 0;
    while (off < size) {
        uint16_t tag = 0;
        const uint8_t *value = NULL;
        size_t value_len = 0;
        auth_err_t rc = auth_tlv_read(data, size, &off, &tag, &value, &value_len);
        if (rc != AUTH_OK) break;
    }
    return 0;
}
