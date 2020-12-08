#include <twr_rtc.h>
#include <twr_irq.h>
#include <stm32l0xx.h>

#define _TWR_RTC_LEAP_YEAR(year) ((((year) % 4 == 0) && ((year) % 100 != 0)) || ((year) % 400 == 0))
#define _TWR_RTC_DAYS_IN_YEAR(x) _TWR_RTC_LEAP_YEAR(x) ? 366 : 365
#define _TWR_RTC_CALENDAR_CENTURY 2000
#define _TWR_RTC_OFFSET_YEAR 1970
#define _TWR_RTC_SECONDS_PER_DAY 86400
#define _TWR_RTC_SECONDS_PER_HOUR 3600
#define _TWR_RTC_SECONDS_PER_MINUTE 60

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


int _twr_rtc_writable_semaphore = 0;

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
 * Pre-computed number of days since the Epoch (January 1st, 1970) for the years
 * from 2000 to 2099 inclusive. The first element correspondings to the year
 * 2000, i.e., days_since_epoch[0] returns the number of days from January 1st,
 * 1970 through December 31st,
 * 1999.
 *
 * The following Python 3 program can be used to generate the contents of the
 * table:
 *
 * from datetime import date epoch = date(1970, 1, 1) for year in range(2000,
 * 2099+1): diff = date(year, 1, 1) - epoch print("0x%x, " % diff.days, end='')
 */
static const uint16_t days_since_epoch[] = {
    0x2acd, 0x2c3b, 0x2da8, 0x2f15, 0x3082, 0x31f0, 0x335d, 0x34ca, 0x3637, 0x37a5,
    0x3912, 0x3a7f, 0x3bec, 0x3d5a, 0x3ec7, 0x4034, 0x41a1, 0x430f, 0x447c, 0x45e9,
    0x4756, 0x48c4, 0x4a31, 0x4b9e, 0x4d0b, 0x4e79, 0x4fe6, 0x5153, 0x52c0, 0x542e,
    0x559b, 0x5708, 0x5875, 0x59e3, 0x5b50, 0x5cbd, 0x5e2a, 0x5f98, 0x6105, 0x6272,
    0x63df, 0x654d, 0x66ba, 0x6827, 0x6994, 0x6b02, 0x6c6f, 0x6ddc, 0x6f49, 0x70b7,
    0x7224, 0x7391, 0x74fe, 0x766c, 0x77d9, 0x7946, 0x7ab3, 0x7c21, 0x7d8e, 0x7efb,
    0x8068, 0x81d6, 0x8343, 0x84b0, 0x861d, 0x878b, 0x88f8, 0x8a65, 0x8bd2, 0x8d40,
    0x8ead, 0x901a, 0x9187, 0x92f5, 0x9462, 0x95cf, 0x973c, 0x98aa, 0x9a17, 0x9b84,
    0x9cf1, 0x9e5f, 0x9fcc, 0xa139, 0xa2a6, 0xa414, 0xa581, 0xa6ee, 0xa85b, 0xa9c9,
    0xab36, 0xaca3, 0xae10, 0xaf7e, 0xb0eb, 0xb258, 0xb3c5, 0xb533, 0xb6a0, 0xb80d
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

void twr_rtc_init(void)
{
}

uint32_t twr_rtc_datetime_to_timestamp(struct tm *tm)
{
    uint32_t days = 0, seconds = 0;
    uint16_t i;
    int year = tm->tm_year + 1900;

    // Year is below offset year
    if (year < _TWR_RTC_OFFSET_YEAR)
    {
        return 0;
    }
    // Days in back years
    for (i = _TWR_RTC_OFFSET_YEAR; i < year; i++)
    {
        days += _TWR_RTC_DAYS_IN_YEAR(i);
    }
    // Days in current year
    days += days_since_new_year[_TWR_RTC_LEAP_YEAR(year)][tm->tm_mon];

    // Day starts with 1
    days += tm->tm_mday - 1;
    seconds = days * _TWR_RTC_SECONDS_PER_DAY;
    seconds += tm->tm_hour * _TWR_RTC_SECONDS_PER_HOUR;
    seconds += tm->tm_min * _TWR_RTC_SECONDS_PER_MINUTE;
    seconds += tm->tm_sec;

    return seconds;
}

void twr_rtc_get_datetime(struct tm *tm)
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
        mem.tm.tm_year = year + _TWR_RTC_CALENDAR_CENTURY - 1900;
        mem.tm.tm_wday = dr.WDU == 7 ? 0 : dr.WDU;
        mem.tm.tm_yday = days_since_new_year[is_leap[year]][mem.tm.tm_mon] + mem.tm.tm_mday - 1;
        mem.dr = dr;
    }
    (*tm) = mem.tm;
}

void twr_rtc_get_timestamp(struct timespec *tv)
{
    // The value 0xFFFFFFFF is a value that the RTC_DR and RTC_TR registers
    // should never have. We use that to trigger a full conversion on the first
    // invocation.
    static struct {
        RTC_DR dr;
        RTC_TR tr;
        time_t s1;
        time_t s2;
    } mem = { .dr.i = 0xFFFFFFFF, .tr.i = 0xFFFFFFFF };

    // FIXME: ssr can be larger than prediv_s after a shift operation. We may
    // need to adjust the date/time in that case. See STM32L0X3 reference
    // manual, section "27.7.10 RTC sub second register"
    RTC_SSR ssr = { .i = RTC->SSR };

    // If RTC_SSR or RTC_TR is read, the value of higher-order date/time
    // registers is frozen until RTC_DR is read. Make sure to read all these
    // registers as quickly as possible before doing more.
    RTC_TR tr = { .i = RTC->TR };
    RTC_DR dr = { .i = RTC->DR };

    // Note: The code generated by the following expression will only be fast if
    // TWR_RTC_PREDIV_S is a power of two.
    tv->tv_nsec = 1000000 * (TWR_RTC_PREDIV_S - 1 - ssr.SS) / TWR_RTC_PREDIV_S;

    if (tr.i != mem.tr.i) {
        mem.s1 = (10 * tr.HT + tr.HU) * _TWR_RTC_SECONDS_PER_HOUR
            + (10 * tr.MNT + tr.MNU) * _TWR_RTC_SECONDS_PER_MINUTE
            + 10 * tr.ST + tr.SU;

        mem.tr = tr;
    }

    // Struct tm counts week days from Sunday starting at 0.
    // RTC counts week days from Mondays starting at 1.
    //       Mo Tu We Th Fr Sa Su
    //   tm  1  2  3  4  5  6  0
    //   rtc 1  2  3  4  5  6  7

    if (dr.i != mem.dr.i) {
        int year = 10 * dr.YT + dr.YU;
        mem.s2 = (days_since_epoch[year]
            + days_since_new_year[is_leap[year]][10 * dr.MT + dr.MU - 1]
            + 10 * dr.DT + dr.DU - 1) * _TWR_RTC_SECONDS_PER_DAY;

        mem.dr = dr;
    }
    tv->tv_sec = mem.s1 + mem.s2;
}

int twr_rtc_set_datetime(struct tm *tm, int ms)
{
    int year = tm->tm_year + 1900 - _TWR_RTC_CALENDAR_CENTURY;
    if (year < 0 || year > 99) return -1;

    int month = tm->tm_mon + 1;

    if (ms < 0 || ms > 999) return -2;
    RTC_SSR ssr = {
        .SS = TWR_RTC_PREDIV_S - ms * (TWR_RTC_PREDIV_S + 1) / 1000
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
        .HU   = tm->tm_hour % 10,
        .HT   = tm->tm_hour / 10,
        .PM   = 0
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

    twr_rtc_enable_write();
    twr_rtc_set_init(true);
    RTC->SSR = ssr.i;
    RTC->TR = tr.i;
    RTC->DR = dr.i;
    twr_rtc_set_init(false);
    twr_rtc_disable_write();
    return 0;
}

void twr_rtc_set_init(bool state)
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
