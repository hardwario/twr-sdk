#include <hio_watchdog.h>

#include <stm32l0xx_hal.h>
#include <stm32l083xx.h>

#ifdef HIO_WATCHDOG_ENABLED
static IWDG_HandleTypeDef _hio_watchdog_hiwdg;

static const uint32_t _hio_watchdog_prescaler[] =
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

void hio_watchdog_init(hio_watchdog_time_t hio_watchdog_time)
{
    (void) hio_watchdog_time;
#ifdef HIO_WATCHDOG_ENABLED
    // T = (IWDG_PRESCALER * 4096) / (38*10^3)
    _hio_watchdog_hiwdg.Instance = IWDG;
    _hio_watchdog_hiwdg.Init.Reload = 0x0FFF;
    _hio_watchdog_hiwdg.Init.Window = 0x0FFF;
    _hio_watchdog_hiwdg.Init.Prescaler = _hio_watchdog_prescaler[hio_watchdog_time];
    HAL_IWDG_Init(&_hio_watchdog_hiwdg);
#endif
}

void hio_watchdog_refresh()
{
#ifdef HIO_WATCHDOG_ENABLED
    HAL_IWDG_Refresh(&_hio_watchdog_hiwdg);
#endif
}

