#include <hio_delay.h>
#include <hio_timer.h>
#include <hio_gpio.h>

void hio_delay_us(uint16_t microseconds)
{
    hio_timer_init();
    hio_timer_start();
    hio_timer_delay(microseconds);
    hio_timer_stop();
}
