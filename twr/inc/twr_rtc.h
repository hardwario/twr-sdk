#ifndef _TWR_RTC_H
#define _TWR_RTC_H

#include <time.h>
#include <stm32l0xx.h>
#include "twr_common.h"

//! @addtogroup twr_rtc twr_rtc
//! @brief Driver for real-time clock
//! @{

// Note: For performance reasons, the TWR_RTC_PREDIV_S value should be a power of
// two.
#define TWR_RTC_PREDIV_S 256
#define TWR_RTC_PREDIV_A 128

//! @brief Initialize real-time clock

extern int _twr_rtc_writable_semaphore;

void twr_rtc_init(void);

/**
 * Obtain current date and time from RTC
 *
 * This function retrieves the current date and time from the RTC peripheral and
 * stores the result broken down in the given struct tm.
 *
 * The function has been optimized for speed. It uses most recent value
 * memoization to amortize run time across successive invocations. Pre-computed
 * tables are used to speed up leap year and year-of-day conversions. Run times
 * measured with 2.1 MHz system clock are as follows:
 *
 *   - 88 us on first use or date register (RTC_DR) change
 *   - 50 us on time register (RTC_TR) change
 *   - 34 us on sub-second register (RTC_SSR) change only
 *
 * Thus, when called repeatedly within a one second interval, the first
 * invocation will complete in 50 us and any subsequent invocations will only
 * take 34 us until the RTC_TR register changes.
 *
 * Warning: The function does not check whether the RTC's shadow registers have
 * been initialized. If invoked in a state where they might not be, e.g., after
 * RTC initialization, system reset, or wake up from deep sleep, you need to
 * perform the check yourself beforehand.
 *
 * The RTC peripheral must be configured in 24-hour mode.
 *
 * @param[out] tm A pointer to target struct tm variable to hold the result
 */
void twr_rtc_get_datetime(struct tm *tm);

/**
 * Obtain current UNIX time from RTC
 *
 * Retrieve the current date and time from the RTC peripheral and return it in
 * the form of a UNIX timestamp. The timestamp is stored in a struct timespec.
 * The attribute tv_sec contains the number of seconds since the Epoch (January
 * 1st, 1970). The attribute tv_nsec contains an additional number of
 * nanoseconds since tv_sec.
 *
 * The resolution of this function depends on the value of the synchronous RTC
 * prescaler. With the default value (256), the resolution is about 4
 * milliseconds.
 *
 * In order for this function to work correctly, the RTC must be configured in
 * the 24-hour mode, must be configured with UTC date and time, and the year
 * must be between 2000 and 2099 (inclusive).
 *
 * The run time of this function depends on the value of the synchronous RTC
 * prescaler register. For optimum performance, the register's value should be a
 * power of two minus one. The run times are as follows with the system clock
 * running at 2.1 MHz:
 *
 *   - 87 us on first use or date register (RTC_DR) change
 *   - 58 us on time register (RTC_TR) change
 *   - 38 us on sub-second register (RTC_SSR) change only
 *
 * The function uses most recent value memoization and pre-computed conversion
 * tables to amortize the run time across successive invocations.
 */
void twr_rtc_get_timestamp(struct timespec *tv);

/**
 * Set date and time in RTC
 *
 * This function configures the current date and time in the RTC peripheral.
 * Date and time is passed broken down (struct tm) in the first parameter. A
 * sub-second component is passed as the number of milliseconds in the second
 * parameter.
 *
 * @param[in] tm Struct tm with calendar date and time to set in the RTC
 * @param[in] ms Sub-second time (number of milliseconds, 0 if unknown)
 * @return 0 on success, a negative number on error
 */
int twr_rtc_set_datetime(struct tm *tm, int ms);

//! @brief Convert date and time to UNIX timestamp
//! @param[in] tm Pointer to the date and time structure
//! @return unix timestamp

uint32_t twr_rtc_datetime_to_timestamp(struct tm *tm);

//! @brief Enable RTC write protection
//
// This function supports nested invocations. If twr_rtc_enable_write has been
// called repeatedly, calling this function repeatedly will only lock the RTC
// again after all calls to twr_rtc_enable_write have been unrolled.

static inline void twr_rtc_disable_write()
{
	if (--_twr_rtc_writable_semaphore <= 0) {
		_twr_rtc_writable_semaphore = 0;
		RTC->WPR = 0xff;
	}
}

//! @brief Disable RTC write protection

static inline void twr_rtc_enable_write()
{
	++_twr_rtc_writable_semaphore;
	RTC->WPR = 0xca;
	RTC->WPR = 0x53;
}

/**
 * Wait for RTC shadow registers to initialize
 *
 * This function blocks until the RTC shadow registers have been initialized,
 * i.e., until the RSF bit in RTC->ISR is set. This function needs to be called
 * after waking up from Stop or Standby modes, before other functions that read
 * the shadow registers such as twr_rt_get_datetime.
 *
 * It takes up to two RTCClk cycles for the shadow registers to initialize.
 */
static inline void twr_rtc_wait()
{
	while(!(RTC->ISR & RTC_ISR_RSF));
}


//! @brief Enable or disable RTC initialization mode
//! @param[in] state Enable when true, disable when false

void twr_rtc_set_init(bool state);

//! @}

#endif // _TWR_RTC_H
