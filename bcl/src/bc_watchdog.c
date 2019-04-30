#include <bc_watchdog.h>

#include <stm32l0xx_hal.h>
#include <stm32l083xx.h>

#ifdef BC_WATCHDOG_ENABLED
static IWDG_HandleTypeDef _bc_watchdog_hiwdg;

static const uint32_t _bc_watchdog_prescaler[] =
{
        IWDG_PRESCALER_256,
        IWDG_PRESCALER_128,
        IWDG_PRESCALER_64,
        IWDG_PRESCALER_32,
        IWDG_PRESCALER_16,
        IWDG_PRESCALER_8,
        IWDG_PRESCALER_4
};
#endif

void bc_watchdog_init(bc_watchdog_time_t bc_watchdog_time)
{
    (void) bc_watchdog_time;
#ifdef BC_WATCHDOG_ENABLED
    // T = (IWDG_PRESCALER * 4096) / (38*10^3)
    _bc_watchdog_hiwdg.Instance = IWDG;
    _bc_watchdog_hiwdg.Init.Reload = 0x0FFF;
    _bc_watchdog_hiwdg.Init.Window = 0x0FFF;
    _bc_watchdog_hiwdg.Init.Prescaler = _bc_watchdog_prescaler[bc_watchdog_time];
    HAL_IWDG_Init(&_bc_watchdog_hiwdg);
#endif
}

void bc_watchdog_refresh()
{
#ifdef BC_WATCHDOG_ENABLED
    HAL_IWDG_Refresh(&_bc_watchdog_hiwdg);
#endif
}

