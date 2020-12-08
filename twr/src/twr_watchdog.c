#include <twr_watchdog.h>

#include <stm32l0xx_hal.h>
#include <stm32l083xx.h>

#ifdef TWR_WATCHDOG_ENABLED
static IWDG_HandleTypeDef _twr_watchdog_hiwdg;

static const uint32_t _twr_watchdog_prescaler[] =
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

void twr_watchdog_init(twr_watchdog_time_t twr_watchdog_time)
{
    (void) twr_watchdog_time;
#ifdef TWR_WATCHDOG_ENABLED
    // T = (IWDG_PRESCALER * 4096) / (38*10^3)
    _twr_watchdog_hiwdg.Instance = IWDG;
    _twr_watchdog_hiwdg.Init.Reload = 0x0FFF;
    _twr_watchdog_hiwdg.Init.Window = 0x0FFF;
    _twr_watchdog_hiwdg.Init.Prescaler = _twr_watchdog_prescaler[twr_watchdog_time];
    HAL_IWDG_Init(&_twr_watchdog_hiwdg);
#endif
}

void twr_watchdog_refresh()
{
#ifdef TWR_WATCHDOG_ENABLED
    HAL_IWDG_Refresh(&_twr_watchdog_hiwdg);
#endif
}

