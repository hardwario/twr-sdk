#ifndef _HIO_SYSTEM_H
#define _HIO_SYSTEM_H

#include <hio_common.h>

typedef enum
{
    HIO_SYSTEM_CLOCK_MSI = 0,
    HIO_SYSTEM_CLOCK_HSI = 1,
    HIO_SYSTEM_CLOCK_PLL = 2

} hio_system_clock_t;

void hio_system_init(void);

void hio_system_sleep(void);

hio_system_clock_t hio_system_clock_get(void);

void hio_system_hsi16_enable(void);

void hio_system_hsi16_disable(void);

void hio_system_pll_enable(void);

void hio_system_pll_disable(void);

void hio_system_deep_sleep_disable(void);

void hio_system_deep_sleep_enable(void);

void hio_system_enter_standby_mode(void);

uint32_t hio_system_get_clock(void);

void hio_system_reset(void);

bool hio_system_get_vbus_sense(void);

#endif // _HIO_SYSTEM_H
