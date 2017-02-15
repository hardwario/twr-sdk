#include "usb_talk.h"
#include <bc_scheduler.h>
#include <bc_gpio.h>
#include <bc_module_power.h>

#define BC_MODULE_POWER_PIN_RELAY BC_GPIO_P0

void bc_module_power_task();

bc_module_power_t bc_module_power;

static struct {
	uint16_t led;
	uint8_t step;

} _bc_module_test;

void bc_module_power_init()
{
	bc_scheduler_disable_sleep();

	memset(&bc_module_power, 0, sizeof(bc_module_power_t));

	bc_module_power.led_strip_on = true;
	bc_module_power.led_strip_count = 144;
	bc_module_power.led_strip_type = BC_WS2812B_TYPE_RGBW;

	bc_gpio_init(BC_MODULE_POWER_PIN_RELAY);
	bc_gpio_set_mode(BC_MODULE_POWER_PIN_RELAY, BC_GPIO_MODE_OUTPUT);

	bc_ws2812b_init(bc_module_power.led_strip_type, bc_module_power.led_strip_count);

	bc_scheduler_register(bc_module_power_task, &bc_module_power, 10);

}

void bc_module_power_led_strip_test(void)
{
	_bc_module_test.led = 0;
	_bc_module_test.step = 0;
	bc_module_power.test = true;
}

bool bc_module_power_print_frame_buffer(uint8_t *frame_buffer, size_t size)
{
	if (size > (size_t)(bc_module_power.led_strip_type * bc_module_power.led_strip_count))
	{
		return false;
	}

	uint16_t column = 0;
	for(int i = 0; i < size; i += bc_module_power.led_strip_type)
	{
		bc_ws2812b_set_pixel(column++,
				frame_buffer[i],
				frame_buffer[i+1],
				frame_buffer[i+2],
				bc_module_power.led_strip_type == BC_WS2812B_TYPE_RGBW ? frame_buffer[i+3] : 0);
	}

	bc_ws2812b_send();

	return true;
}

void bc_module_power_task()
{

	if (bc_ws2812b_send() & bc_module_power.test)
	{

		uint8_t tint = 255 * _bc_module_test.led / bc_module_power.led_strip_count;

		if (_bc_module_test.step == 0)
		{
			bc_ws2812b_set_pixel( _bc_module_test.led, tint, 0, 0, 0);
		}
		else if (_bc_module_test.step == 1)
		{
			bc_ws2812b_set_pixel(_bc_module_test.led, 0, tint, 0, 0);
		}
		else if (_bc_module_test.step == 2)
		{
			bc_ws2812b_set_pixel(_bc_module_test.led, 0, 0, tint, 0);
		}
		else if (_bc_module_test.step == 3)
		{
			if (bc_module_power.led_strip_type == BC_WS2812B_TYPE_RGB)
			{
				bc_ws2812b_set_pixel(_bc_module_test.led, tint, tint, tint, 0);
			}
			else
			{
				bc_ws2812b_set_pixel(_bc_module_test.led, 0, 0, 0, tint);
			}
		}
		else
		{
			bc_ws2812b_set_pixel(_bc_module_test.led, 0, 0, 0, 0);
		}

		_bc_module_test.led++;

		if (_bc_module_test.led == bc_module_power.led_strip_count)
		{
			_bc_module_test.led = 0;
			_bc_module_test.step++;
			if (_bc_module_test.step == 5)
			{
				bc_module_power.test = false;
			}
		}
	}

	bc_gpio_set_output(BC_MODULE_POWER_PIN_RELAY, bc_module_power.relay_is_on);

	bc_scheduler_plan_current_relative(10);
}
