#ifndef _BC_WATCHDOG_H
#define _BC_WATCHDOG_H

//#define BC_WATCHDOG_ENABLED

typedef enum
{
    BC_WATCHDOG_27S = 0,
    BC_WATCHDOG_13S = 1,
    BC_WATCHDOG_6S = 2,
    BC_WATCHDOG_3S = 3,
    BC_WATCHDOG_1S = 4,
    BC_WATCHDOG_86MS = 5,
    BC_WATCHDOG_43MS = 6,
} bc_watchdog_time_t;

//! @brief Initialize Independent Watchdog based on LSI oscillator
//! @param[in] bc_watchdog_time Time when the watchdog needs to be refreshed

void bc_watchdog_init(bc_watchdog_time_t bc_watchdog_time);

//! @brief Refresh watchdog so that the mcu is not restarted

void bc_watchdog_refresh();

#endif