#include <bc_delay.h>
#include <bc_timer.h>
#include <bc_gpio.h>

void bc_delay_us(uint16_t microseconds)
{
    bc_timer_init();
    bc_timer_start();
    bc_timer_delay(microseconds);
    bc_timer_stop();
}
