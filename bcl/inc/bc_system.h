#ifndef _BC_SYSTEM_H
#define _BC_SYSTEM_H

#include <bc_common.h>

void bc_system_init(void);

void bc_system_sleep(void);

void bc_system_pll_enable(void);

void bc_system_pll_disable(void);

void bc_system_deep_sleep_disable(void);

void bc_system_deep_sleep_enable(void);

uint32_t bc_system_get_clock(void);

void bc_system_reset(void);

#endif // _BC_SYSTEM_H
