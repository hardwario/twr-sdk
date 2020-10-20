#include <bc_rtc.h>
#include <bc_irq.h>
#include <stm32l0xx.h>

#define _BC_RTC_LEAP_YEAR(year) ((((year) % 4 == 0) && ((year) % 100 != 0)) || ((year) % 400 == 0))
#define _BC_RTC_DAYS_IN_YEAR(x) _BC_RTC_LEAP_YEAR(x) ? 366 : 365
#define _BC_RTC_CALENDAR_CENTURY 2000
#define _BC_RTC_OFFSET_YEAR 1970
#define _BC_RTC_SECONDS_PER_DAY 86400
#define _BC_RTC_SECONDS_PER_HOUR 3600
#define _BC_RTC_SECONDS_PER_MINUTE 60

/**
 * Accessor data type for RTC_TR
 *
 * This type allows accessing the contents of RTC_TR either as a 32-bit integer,
 * or by individual bit fields without the need to mask and shift the value. See
 * the STM32L0x3 Reference Manual, Section 27.7.1 RTC time register (RTC_TR) for
 * the description of individual bit fields.
 */
typedef union {
    uint32_t i;
    struct {
        unsigned int SU   : 4;
        unsigned int ST   : 3;
        unsigned int res1 : 1;
        unsigned int MNU  : 4;
        unsigned int MNT  : 3;
        unsigned int res2 : 1;
        unsigned int HU   : 4;
        unsigned int HT   : 2;
        unsigned int PM   : 1;
    };
} RTC_TR;

/**
 * Accessor data type for RTC_DR
 *
 * This type allows accessing the contents of RTC_DR either as a 32-bit integer,
 * or by individual bit fields without the need to mask and shift the value. See
 * the STM32L0x3 Reference Manual, Section 27.7.2 RTC date register (RTC_DR) for
 * the description of individual bit fields.
 */
typedef union {
    uint32_t i;
    struct {
        unsigned int DU  : 4;
        unsigned int DT  : 2;
        unsigned int res : 2;
        unsigned int MU  : 4;
        unsigned int MT  : 1;
        unsigned int WDU : 3;
        unsigned int YU  : 4;
        unsigned int YT  : 4;
    };
} RTC_DR;

/**
 * Accessor data type for RTC_SSR
 *
 * This type allows accessing the contents of RTC_SSR either as a 32-bit
 * integer, or by the sub-section field. See the STM32L0x3 Reference Manual,
 * Section 27.7.10 RTC sub section register (RTC_SSR) for more information on
 * the SS field.
 */
typedef union {
    uint32_t i;
    uint16_t SS;
} RTC_SSR;


int _bc_rtc_writable_semaphore = 0;

/*
 * The following table can be used to quickly determine whether a particular
 * year is a leap year. The index to the array is the year number within the
 * century, e.g., is_leap[0] returns the leap year information for the year
 * 2000.
 *
 * The following Python 3 program can be used to generate the contents of the
 * table:
 *
 * from calendar import isleap
 * from datetime import date
 * for year in range(2000, 2099+1):
 *    print("%d, " % isleap(year), end='')
 */
static const uint8_t is_leap[] = {
    1, 0, 0, 0, 1, 0, 0, 0, 1, 0,
    0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    1, 0, 0, 0, 1, 0, 0, 0, 1, 0,
    0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    1, 0, 0, 0, 1, 0, 0, 0, 1, 0,
    0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    1, 0, 0, 0, 1, 0, 0, 0, 1, 0,
    0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
    1, 0, 0, 0, 1, 0, 0, 0, 1, 0,
    0, 0, 1, 0, 0, 0, 1, 0, 0, 0
};

/*
 * The following table can be used to quickly retrieve the number of days since
 * January 1st until the 1st day of the given month (exclusive). Two variants
 * are provided for regular and leap years. E.g., days_since_new_year[0][5]
 * returns the number of days from January 1st through April 30th in a regular
 * (non-leap) year.
 */
static const uint16_t days_since_new_year[2][12] = {
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334}, // Regular year
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}  // Leap year
};

void bc_rtc_init(void)
{
}

uint32_t bc_rtc_rtc_to_timestamp(bc_rtc_t *rtc)
{
    uint32_t days = 0, seconds = 0;
    uint16_t i;
    // Year is below offset year
    if (rtc->year < _BC_RTC_OFFSET_YEAR)
    {
        return 0;
    }
    // Days in back years
    for (i = _BC_RTC_OFFSET_YEAR; i < rtc->year; i++)
    {
        days += _BC_RTC_DAYS_IN_YEAR(i);
    }
    // Days in current year
    days += days_since_new_year[_BC_RTC_LEAP_YEAR(rtc->year)][rtc->month - 1];

    // Day starts with 1
    days += rtc->date - 1;
    seconds = days * _BC_RTC_SECONDS_PER_DAY;
    seconds += rtc->hours * _BC_RTC_SECONDS_PER_HOUR;
    seconds += rtc->minutes * _BC_RTC_SECONDS_PER_MINUTE;
    seconds += rtc->seconds;

    return seconds;
}

void bc_rtc_get_datetime(struct tm *tm)
{
    // The value 0xFFFFFFFF is a value that the RTC_DR and RTC_TR registers
    // should never have. We use that to trigger a full conversion on the first
    // invocation.
    //
    // The RTC does not keep track of daylight saving time, so we set the
    // corresponding field to -1.
    static struct {
        RTC_DR dr;
        RTC_TR tr;
        struct tm tm;
    } mem = { .dr.i = 0xFFFFFFFF, .tr.i = 0xFFFFFFFF, .tm.tm_isdst = -1 };

    // If RTC_SSR or RTC_TR is read, the value of higher-order date/time
    // registers is frozen until RTC_DR is read. Make sure to read all these
    // registers as quickly as possible before doing more.
    RTC_TR tr = { .i = RTC->TR };
    RTC_DR dr = { .i = RTC->DR };

    if (tr.i != mem.tr.i) {
        mem.tm.tm_sec = 10 * tr.ST + tr.SU;
        mem.tm.tm_min = 10 * tr.MNT + tr.MNU;
        mem.tm.tm_hour = 10 * tr.HT + tr.HU;
        // If the RTC is configured in AM/PM mode and the PM bit is set, convert the
        // PM value to a 24-hour value.
        if ((RTC->CR & RTC_CR_FMT) && tr.PM) mem.tm.tm_hour += 12;
        mem.tr = tr;
    }

    // Struct tm counts week days from Sunday starting at 0.
    // RTC counts week days from Mondays starting at 1.
    //       Mo Tu We Th Fr Sa Su
    //   tm  1  2  3  4  5  6  0
    //   rtc 1  2  3  4  5  6  7

    if (dr.i != mem.dr.i) {
        int year = 10 * dr.YT + dr.YU;
        mem.tm.tm_mday = 10 * dr.DT + dr.DU;
        mem.tm.tm_mon  = 10 * dr.MT + dr.MU - 1;
        mem.tm.tm_year = year + _BC_RTC_CALENDAR_CENTURY - 1900;
        mem.tm.tm_wday = dr.WDU == 7 ? 0 : dr.WDU;
        mem.tm.tm_yday = days_since_new_year[is_leap[year]][mem.tm.tm_mon] + mem.tm.tm_mday - 1;
        mem.dr = dr;
    }
    (*tm) = mem.tm;
}

int bc_rtc_set_datetime(struct tm *tm, int ms)
{
    int year = tm->tm_year + 1900 - _BC_RTC_CALENDAR_CENTURY;
    if (year < 0 || year > 99) return -1;

    int month = tm->tm_mon + 1;

    int hour, pm;
    if ((RTC->CR & RTC_CR_FMT) && (tm->tm_hour > 12)) {
        hour = tm->tm_hour - 12;
        pm = 1;
    } else {
        hour = tm->tm_hour;
        pm = 0;
    }

    if (ms < 0 || ms > 999) return -2;
    RTC_SSR ssr = {
        .SS = BC_RTC_PREDIV_S - ms * (BC_RTC_PREDIV_S + 1) / 1000
    };

    // Note: The RTC datasheet says that the values of the two reserved bits
    // must be left set at their reset values. To err on the side of caution, we
    // fetch those values from the RTC and preserve them when modifying the
    // register value.
    RTC_TR old_tr = { .i = RTC->TR };

    RTC_TR tr = {
        .SU   = tm->tm_sec % 10,
        .ST   = tm->tm_sec / 10,
        .res1 = old_tr.res1,
        .MNU  = tm->tm_min % 10,
        .MNT  = tm->tm_min / 10,
        .res2 = old_tr.res2,
        .HU   = hour % 10,
        .HT   = hour / 10,
        .PM   = pm
    };

    // Struct tm counts week days from Sunday starting at 0.
    // RTC counts week days from Mondays starting at 1.
    //       Mo Tu We Th Fr Sa Su
    //   tm  1  2  3  4  5  6  0
    //   rtc 1  2  3  4  5  6  7

    RTC_DR old_dr = { .i = RTC->DR };
    RTC_DR dr = {
        .DU  = tm->tm_mday % 10,
        .DT  = tm->tm_mday / 10,
        .res = old_dr.res,
        .MU  = month % 10,
        .MT  = month / 10,
        .WDU = tm->tm_wday == 0 ? 7 : tm->tm_wday,
        .YU  = year % 10,
        .YT  = year / 10,
    };

    bc_rtc_enable_write();
    bc_rtc_set_init(true);
    RTC->SSR = ssr.i;
    RTC->TR = tr.i;
    RTC->DR = dr.i;
    bc_rtc_set_init(false);
    bc_rtc_disable_write();
    return 0;
}

void bc_rtc_set_init(bool state)
{
    if (state) {
        // Enable initialization mode
        RTC->ISR |= RTC_ISR_INIT;

        // Wait for RTC to be in initialization mode...
        while ((RTC->ISR & RTC_ISR_INITF) == 0)
        {
            continue;
        }
    } else {
        // Exit from initialization mode
        RTC->ISR &= ~RTC_ISR_INIT;
    }
}
