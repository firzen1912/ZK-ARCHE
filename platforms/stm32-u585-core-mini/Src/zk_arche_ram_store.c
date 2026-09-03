#include "zk_arche_ram_store.h"

#include "auth/auth_crypto.h"

#include <string.h>

void zk_stm32_ram_store_init(zk_stm32_ram_store_t *store)
{
    if (store == NULL) return;
    auth_zeroize(store, sizeof *store);
}

auth_err_t zk_stm32_ram_creds_load(
    void *ctx, auth_credentials_t *creds, int *found)
{
    if (ctx == NULL || creds == NULL || found == NULL) {
        return AUTH_ERR_INVALID_ARGUMENT;
    }

    zk_stm32_ram_store_t *store = (zk_stm32_ram_store_t *)ctx;
    memset(creds, 0, sizeof *creds);
    *found = store->valid;
    if (store->valid) memcpy(creds, &store->creds, sizeof *creds);
    return AUTH_OK;
}

auth_err_t zk_stm32_ram_creds_save(
    void *ctx, const auth_credentials_t *creds)
{
    if (ctx == NULL || creds == NULL) return AUTH_ERR_INVALID_ARGUMENT;

    zk_stm32_ram_store_t *store = (zk_stm32_ram_store_t *)ctx;
    memcpy(&store->creds, creds, sizeof store->creds);
    store->valid = 1;
    return AUTH_OK;
}
