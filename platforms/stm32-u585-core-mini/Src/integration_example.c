/*
 * Example integration for a CubeMX-generated STM32U585CIU6 project.
 *
 * Add this file only after CubeMX has generated huart1 and hrng. Call
 * zk_arche_demo_run() once after HAL_Init(), clock setup, GPIO, USART1 and RNG
 * initialization. The RAM store is bring-up-only and loses identity on reset.
 */

#include "zk_arche_stm32.h"
#include "zk_arche_ram_store.h"

extern UART_HandleTypeDef huart1;
extern RNG_HandleTypeDef hrng;

static zk_stm32_ram_store_t s_store;

/* Set to a non-empty value when the Linux responder is started with
 * --require-pairing-token. Keep TOFU disabled for the default example. */
static const uint8_t s_pairing_token[] = "zk-arche-lab-token";

void zk_arche_demo_run(void)
{
    zk_stm32_ram_store_init(&s_store);

    zk_stm32_client_config_t cfg = {
        .uart = &huart1,
        .rng = &hrng,
        .load_creds = zk_stm32_ram_creds_load,
        .save_creds = zk_stm32_ram_creds_save,
        .store_ctx = &s_store,
        .requested_role = 2,
        .allowed_roles = {1, 2},
        .n_allowed_roles = 2,
        .pairing_token = s_pairing_token,
        .pairing_token_len = sizeof s_pairing_token - 1u,
        .allow_tofu_setup = 0,
        .uart_timeout_ms = 5000,
    };

    auth_err_t result = zk_stm32_client_run(&cfg);

    /* Put a breakpoint here during initial bring-up. Do not print secrets.
     * result == AUTH_OK means SETUP+AUTH completed during this boot. */
    if (result == AUTH_OK) {
        __NOP();
    } else {
        __BKPT(0);
    }
}
