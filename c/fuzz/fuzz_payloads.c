#include <stddef.h>
#include <stdint.h>
#include "auth/auth_payloads.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auth_setup1_t s1;
    auth_setup2_t s2;
    auth_setup3_t s3;
    auth_auth1_t a1;
    auth_auth2_t a2;
    auth_auth3_t a3;
    (void)auth_setup1_decode(&s1, data, size);
    (void)auth_setup2_decode(&s2, data, size);
    (void)auth_setup3_decode(&s3, data, size);
    (void)auth_auth1_decode(&a1, data, size);
    (void)auth_auth2_decode(&a2, data, size);
    (void)auth_auth3_decode(&a3, data, size);
    (void)auth_ack_decode(data, size);
    return 0;
}
