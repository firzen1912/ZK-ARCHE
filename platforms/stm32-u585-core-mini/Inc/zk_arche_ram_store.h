#ifndef ZK_ARCHE_RAM_STORE_H
#define ZK_ARCHE_RAM_STORE_H

#include "zk_arche_stm32.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Bring-up-only credential backend.
 *
 * State survives multiple SETUP/AUTH operations in the same boot but is lost
 * on reset/power loss. It exists so the protocol, UART bridge, RNG and crypto
 * stack can be flashed and validated before a linker-reserved flash journal is
 * introduced. It MUST NOT be used as restart/replay/persistent-identity
 * qualification evidence.
 */
typedef struct zk_stm32_ram_store {
    auth_credentials_t creds;
    int valid;
} zk_stm32_ram_store_t;

void zk_stm32_ram_store_init(zk_stm32_ram_store_t *store);

auth_err_t zk_stm32_ram_creds_load(
    void *ctx, auth_credentials_t *creds, int *found);

auth_err_t zk_stm32_ram_creds_save(
    void *ctx, const auth_credentials_t *creds);

#ifdef __cplusplus
}
#endif

#endif /* ZK_ARCHE_RAM_STORE_H */
