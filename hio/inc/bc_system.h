#ifndef _BC_SYSTEM_H
#define _BC_SYSTEM_H

#include <bc_common.h>

typedef enum
{
    BC_SYSTEM_CLOCK_MSI = 0,
    BC_SYSTEM_CLOCK_HSI = 1,
    BC_SYSTEM_CLOCK_PLL = 2

} bc_system_clock_t;

void bc_system_init(void);

void bc_system_sleep(void);

bc_system_clock_t bc_system_clock_get(void);

void bc_system_hsi16_enable(void);

void bc_system_hsi16_disable(void);

void bc_system_pll_enable(void);

void bc_system_pll_disable(void);

void bc_system_deep_sleep_disable(void);

void bc_system_deep_sleep_enable(void);

void bc_system_enter_standby_mode(void);

uint32_t bc_system_get_clock(void);

void bc_system_reset(void);

bool bc_system_get_vbus_sense(void);

#endif // _BC_SYSTEM_H
