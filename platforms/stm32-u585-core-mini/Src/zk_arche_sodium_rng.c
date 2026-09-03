#include "zk_arche_stm32.h"

#include <sodium.h>
#include <stdint.h>
#include <string.h>

static RNG_HandleTypeDef *s_hrng;

static void rng_fail_closed(void)
{
    __disable_irq();
    for (;;) { __NOP(); }
}

static const char *stm32_rng_name(void)
{
    return "stm32u5-hal-rng";
}

static uint32_t stm32_rng_random(void)
{
    uint32_t v = 0;
    if (s_hrng == NULL || HAL_RNG_GenerateRandomNumber(s_hrng, &v) != HAL_OK) {
        rng_fail_closed();
    }
    return v;
}

static void stm32_rng_buf(void * const buf, const size_t size)
{
    uint8_t *out = (uint8_t *)buf;
    size_t offset = 0;
    while (offset < size) {
        uint32_t v = stm32_rng_random();
        size_t take = size - offset;
        if (take > sizeof v) take = sizeof v;
        memcpy(out + offset, &v, take);
        offset += take;
    }
}

static const randombytes_implementation s_rng_impl = {
    .implementation_name = stm32_rng_name,
    .random = stm32_rng_random,
    .stir = NULL,
    .uniform = NULL,
    .buf = stm32_rng_buf,
    .close = NULL,
};

auth_err_t zk_stm32_install_sodium_rng(RNG_HandleTypeDef *hrng)
{
    if (hrng == NULL) return AUTH_ERR_INVALID_ARGUMENT;
    s_hrng = hrng;
    return randombytes_set_implementation(&s_rng_impl) == 0
        ? AUTH_OK : AUTH_ERR_NOT_INITIALIZED;
}
