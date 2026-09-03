#ifndef ZK_ARCHE_STM32_H
#define ZK_ARCHE_STM32_H

#include "stm32u5xx_hal.h"
#include "auth/auth.h"
#include "auth/store.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef auth_err_t (*zk_stm32_creds_load_fn)(
    void *ctx, auth_credentials_t *creds, int *found);

typedef auth_err_t (*zk_stm32_creds_save_fn)(
    void *ctx, const auth_credentials_t *creds);

typedef struct zk_stm32_client_config {
    UART_HandleTypeDef *uart;
    RNG_HandleTypeDef *rng;

    zk_stm32_creds_load_fn load_creds;
    zk_stm32_creds_save_fn save_creds;
    void *store_ctx;

    uint64_t requested_role;
    uint64_t allowed_roles[AUTH_MAX_ROLES];
    size_t n_allowed_roles;

    const uint8_t *pairing_token;
    size_t pairing_token_len;
    int allow_tofu_setup;

    uint32_t uart_timeout_ms;
} zk_stm32_client_config_t;

/* Install the STM32 hardware RNG as libsodium's randombytes provider.
 * Must be called before auth_init()/sodium_init(). */
auth_err_t zk_stm32_install_sodium_rng(RNG_HandleTypeDef *hrng);

/* Execute explicit SETUP when credentials are incomplete, persist the
 * resulting state, then execute normal AUTH. The UART carries raw ZK-ARCHE
 * packets with a two-byte big-endian length prefix; a byte-forwarding peer or
 * bridge must not make authentication/authorization decisions on behalf of
 * this device. */
auth_err_t zk_stm32_client_run(const zk_stm32_client_config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif /* ZK_ARCHE_STM32_H */
