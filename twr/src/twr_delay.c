#include <twr_delay.h>
#include <twr_timer.h>
#include <twr_gpio.h>

void twr_delay_us(uint16_t microseconds)
{
    twr_timer_init();
    twr_timer_start();
    twr_timer_delay(microseconds);
    twr_timer_stop();
}
