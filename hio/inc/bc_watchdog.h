#ifndef _HIO_WATCHDOG_H
#define _HIO_WATCHDOG_H

//#define HIO_WATCHDOG_ENABLED

typedef enum
{
    HIO_WATCHDOG_27S = 0,
    HIO_WATCHDOG_13S = 1,
    HIO_WATCHDOG_6S = 2,
    HIO_WATCHDOG_3S = 3,
    HIO_WATCHDOG_1S = 4,
    HIO_WATCHDOG_86MS = 5,
    HIO_WATCHDOG_43MS = 6,
} hio_watchdog_time_t;

//! @brief Initialize Independent Watchdog based on LSI oscillator
//! @param[in] hio_watchdog_time Time when the watchdog needs to be refreshed

void hio_watchdog_init(hio_watchdog_time_t hio_watchdog_time);

//! @brief Refresh watchdog so that the mcu is not restarted

void hio_watchdog_refresh();

#endif
