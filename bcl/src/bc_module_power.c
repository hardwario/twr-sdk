#include <bc_module_power.h>
#include <bc_scheduler.h>
#include <bc_gpio.h>


#define BC_MODULE_POWER_PIN_RELAY BC_GPIO_P0

void bc_module_power_task();

uint8_t _bc_module_dma_bit_buffer[144 * 4 * 8];

static struct
{
    bool relay_is_on;
    bool led_strip_on;
    uint16_t led_strip_count;
    bc_ws2812b_type_t led_strip_type;
    bool test;

} _bc_module_power;

static struct {
	uint16_t led;
	uint8_t step;

} _bc_module_test;

void bc_module_power_init(void)
{
	bc_scheduler_disable_sleep();

	_bc_module_power.led_strip_on = true;
	_bc_module_power.led_strip_count = 144;
	_bc_module_power.led_strip_type = BC_WS2812B_TYPE_RGBW;

	bc_gpio_init(BC_MODULE_POWER_PIN_RELAY);
	bc_gpio_set_mode(BC_MODULE_POWER_PIN_RELAY, BC_GPIO_MODE_OUTPUT);

	bc_ws2812b_init(_bc_module_dma_bit_buffer, _bc_module_power.led_strip_type, _bc_module_power.led_strip_count);

	bc_scheduler_register(bc_module_power_task, &_bc_module_power, 10);

}

void bc_module_power_led_strip_test(void)
{
	_bc_module_test.led = 0;
	_bc_module_test.step = 0;
	_bc_module_power.test = true;
}

bool bc_module_power_set_led_strip(const uint8_t *frame_buffer, size_t length)
{
	if (length > (size_t)(_bc_module_power.led_strip_type * _bc_module_power.led_strip_count))
	{
		return false;
	}

	uint16_t position = 0;
	for(uint16_t i = 0; i < length; i += _bc_module_power.led_strip_type)
	{
		bc_ws2812b_set_pixel(position++,
				frame_buffer[i],
				frame_buffer[i+1],
				frame_buffer[i+2],
				_bc_module_power.led_strip_type == BC_WS2812B_TYPE_RGBW ? frame_buffer[i+3] : 0);
	}

	bc_ws2812b_send();

	return true;
}

void bc_module_power_set_relay(bool state)
{
	_bc_module_power.relay_is_on = state;
	bc_gpio_set_output(BC_MODULE_POWER_PIN_RELAY, _bc_module_power.relay_is_on);
}

bool bc_module_power_get_relay()
{
	return _bc_module_power.relay_is_on;
}

void bc_module_power_led_strip_set_pixel(uint16_t position, uint8_t red, uint8_t green, uint8_t blue, uint8_t white)
{
	bc_ws2812b_set_pixel(position, red, green, blue, white);
}

void bc_module_power_task()
{

	if (bc_ws2812b_send() & _bc_module_power.test)
	{

		uint8_t tint = 255 * (_bc_module_test.led + 1) / (_bc_module_power.led_strip_count + 1);

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
			if (_bc_module_power.led_strip_type == BC_WS2812B_TYPE_RGB)
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

		if (_bc_module_test.led == _bc_module_power.led_strip_count)
		{
			_bc_module_test.led = 0;
			_bc_module_test.step++;
			if (_bc_module_test.step == 5)
			{
				_bc_module_power.test = false;
			}
		}
	}

	bc_scheduler_plan_current_relative(10);
}
