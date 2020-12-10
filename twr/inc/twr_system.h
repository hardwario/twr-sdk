#ifndef _TWR_SYSTEM_H
#define _TWR_SYSTEM_H

#include <stm32l0xx.h>
#include <twr_common.h>

typedef enum
{
    TWR_SYSTEM_CLOCK_MSI = 0,
    TWR_SYSTEM_CLOCK_HSI = 1,
    TWR_SYSTEM_CLOCK_PLL = 2

} twr_system_clock_t;

void twr_system_init(void);

static inline void twr_system_sleep(void)
{
    __WFI();

    // TODO: Is there a better way to determine whether the RTC RSF bit needs to
    // be cleared?

    if (SCB->SCR & SCB_SCR_SLEEPDEEP_Msk) {
        /* RTC shadow registers are not updated in deep sleep modes. When waking
         * up from deep sleep, clear the RSF bit in RTC->ISR so that functions
         * that read the RTC calendar know whether they need to wait 2 RTCCLK
         * periods for the registers to re-initialize.
         *
         * Note: We intentially do NOT use twr_rtc_{enable,disable}_write here
         * for performance reasons. This function is on the scheduler hot path
         * and should be as quick as possible. A minor drawback is that if the
         * application leaves the RTC open for writes when control returns to
         * the main scheduler function, writes to RTC will be disabled
         * regardless of the _twr_rtc_writable_semaphore values. Leaving the RTC
         * open for writes is considered a bug anayway.
         */
        RTC->WPR = 0xca;
        RTC->WPR = 0x53;
        RTC->ISR &= ~RTC_ISR_RSF;
        RTC->WPR = 0xff;
    }
}

twr_system_clock_t twr_system_clock_get(void);

void twr_system_hsi16_enable(void);

void twr_system_hsi16_disable(void);

void twr_system_pll_enable(void);

void twr_system_pll_disable(void);

void twr_system_deep_sleep_disable(void);

void twr_system_deep_sleep_enable(void);

void twr_system_enter_standby_mode(void);

uint32_t twr_system_get_clock(void);

void twr_system_reset(void);

bool twr_system_get_vbus_sense(void);

#endif // _TWR_SYSTEM_H
