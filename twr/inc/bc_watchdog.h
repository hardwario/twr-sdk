#ifndef _TWR_WATCHDOG_H
#define _TWR_WATCHDOG_H

//#define TWR_WATCHDOG_ENABLED

typedef enum
{
    TWR_WATCHDOG_27S = 0,
    TWR_WATCHDOG_13S = 1,
    TWR_WATCHDOG_6S = 2,
    TWR_WATCHDOG_3S = 3,
    TWR_WATCHDOG_1S = 4,
    TWR_WATCHDOG_86MS = 5,
    TWR_WATCHDOG_43MS = 6,
} twr_watchdog_time_t;

//! @brief Initialize Independent Watchdog based on LSI oscillator
//! @param[in] twr_watchdog_time Time when the watchdog needs to be refreshed

void twr_watchdog_init(twr_watchdog_time_t twr_watchdog_time);

//! @brief Refresh watchdog so that the mcu is not restarted

void twr_watchdog_refresh();

#endif
