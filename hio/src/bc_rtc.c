#include <bc_rtc.h>
#include <bc_irq.h>
#include <stm32l0xx.h>

#define _BC_RTC_LEAP_YEAR(year) ((((year) % 4 == 0) && ((year) % 100 != 0)) || ((year) % 400 == 0))
#define _BC_RTC_DAYS_IN_YEAR(x) _BC_RTC_LEAP_YEAR(x) ? 366 : 365
#define _BC_RTC_OFFSET_YEAR 1970
#define _BC_RTC_SECONDS_PER_DAY 86400
#define _BC_RTC_SECONDS_PER_HOUR 3600
#define _BC_RTC_SECONDS_PER_MINUTE 60

// Days in a month
uint8_t _bc_rtc_months[2][12] =
    {
        {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, // Not leap year
        {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}  // Leap year
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
    for (i = 1; i < rtc->month; i++)
    {
        days += _bc_rtc_months[_BC_RTC_LEAP_YEAR(rtc->year)][i - 1];
    }
    // Day starts with 1
    days += rtc->date - 1;
    seconds = days * _BC_RTC_SECONDS_PER_DAY;
    seconds += rtc->hours * _BC_RTC_SECONDS_PER_HOUR;
    seconds += rtc->minutes * _BC_RTC_SECONDS_PER_MINUTE;
    seconds += rtc->seconds;

    return seconds;
}

void bc_rtc_get_date_time(bc_rtc_t *rtc)
{
    uint32_t unix;

    // Format hours
    uint32_t tr = RTC->TR;
    rtc->hours = 10 * ((tr & RTC_TR_HT_Msk) >> RTC_TR_HT_Pos) + ((tr & RTC_TR_HU_Msk) >> RTC_TR_HU_Pos);
    rtc->minutes = 10 * ((tr & RTC_TR_MNT_Msk) >> RTC_TR_MNT_Pos) + ((tr & RTC_TR_MNU_Msk) >> RTC_TR_MNU_Pos);
    rtc->seconds = 10 * ((tr & RTC_TR_ST_Msk) >> RTC_TR_ST_Pos) + ((tr & RTC_TR_SU_Msk) >> RTC_TR_SU_Pos);

    // Get subseconds
    rtc->subseconds = RTC->SSR;

    // Format date
    uint32_t dr = RTC->DR;
    rtc->year = 2000 + 10 * ((dr & RTC_DR_YT_Msk) >> RTC_DR_YT_Pos) + ((dr & RTC_DR_YU_Msk) >> RTC_DR_YU_Pos);
    rtc->month = 10 * ((dr & RTC_DR_MT_Msk) >> RTC_DR_MT_Pos) + ((dr & RTC_DR_MU_Msk) >> RTC_DR_MU_Pos);
    rtc->date = 10 * ((dr & RTC_DR_DT_Msk) >> RTC_DR_DT_Pos) + ((dr & RTC_DR_DU_Msk) >> RTC_DR_DU_Pos);
    rtc->week_day = (dr & RTC_DR_WDU_Msk) >> RTC_DR_WDU_Pos;

    // Calculate unix offset
    unix = bc_rtc_rtc_to_timestamp(rtc);
    rtc->timestamp = unix;
}

bool bc_rtc_set_date_time(bc_rtc_t *rtc)
{
    if (rtc->year < 2000 || rtc->year > 2099)
    {
        return false;
    }

    // Disable write protection
    RTC->WPR = 0xca;
    RTC->WPR = 0x53;

    // Enable Init Mode
    RTC->ISR |= RTC_ISR_INIT;

    while ((RTC->ISR & RTC_ISR_INITF) == 0)
    {
        continue;
    }

    // Format hours
    uint32_t tr;
    tr = (rtc->hours / 10) << RTC_TR_HT_Pos |
         (rtc->hours % 10) << RTC_TR_HU_Pos |
         (rtc->minutes / 10) << RTC_TR_MNT_Pos |
         (rtc->minutes % 10) << RTC_TR_MNU_Pos |
         (rtc->seconds / 10) << RTC_TR_ST_Pos |
         (rtc->seconds % 10) << RTC_TR_SU_Pos;

    RTC->TR = tr;

    // Format date
    uint32_t dr;

    dr = ((rtc->year - 2000) / 10) << RTC_DR_YT_Pos |
         ((rtc->year - 2000) % 10) << RTC_DR_YU_Pos |
         (rtc->month / 10) << RTC_DR_MT_Pos |
         (rtc->month % 10) << RTC_DR_MU_Pos |
         (rtc->date / 10) << RTC_DR_DT_Pos |
         (rtc->date % 10) << RTC_DR_DU_Pos;

    RTC->DR = dr;

    // Disable Init Mode
    RTC->ISR &= ~RTC_ISR_INIT;

    // Enable Write Protection
    RTC->WPR = 0xff;

    return true;
}
