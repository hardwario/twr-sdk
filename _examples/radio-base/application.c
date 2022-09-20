#include <application.h>

// LED instance
twr_led_t led;

uint64_t my_did;
uint64_t peer_id;

// Button instance
twr_button_t button;
twr_button_t button_5s;

void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == TWR_BUTTON_EVENT_HOLD)
    {
        twr_radio_pairing_mode_start();

        twr_led_set_mode(&led, TWR_LED_MODE_BLINK_FAST);
    }
}

static void button_5s_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
	(void) self;
	(void) event_param;

	if (event == TWR_BUTTON_EVENT_HOLD)
	{
		twr_radio_pairing_mode_stop();

		uint64_t devices_address[TWR_RADIO_MAX_DEVICES];

		// Read all remote address
		twr_radio_get_peer_id(devices_address, TWR_RADIO_MAX_DEVICES);

		for (int i = 0; i < TWR_RADIO_MAX_DEVICES; i++)
		{
			if (devices_address[i] != 0)
			{
				// Remove device
			    twr_radio_peer_device_remove(devices_address[i]);
			}
		}

		twr_led_pulse(&led, 2000);
	}
}

void radio_event_handler(twr_radio_event_t event, void *event_param)
{
    (void) event_param;

    twr_led_set_mode(&led, TWR_LED_MODE_OFF);

    peer_id = twr_radio_get_event_id();

    if (event == TWR_RADIO_EVENT_ATTACH)
    {
        twr_led_pulse(&led, 1000);
    }
    else if (event == TWR_RADIO_EVENT_DETACH)
	{
		twr_led_pulse(&led, 3000);
	}
    else if (event == TWR_RADIO_EVENT_ATTACH_FAILURE)
    {
        twr_led_pulse(&led, 10000);
    }

    else if (event == TWR_RADIO_EVENT_INIT_DONE)
    {
        my_did = twr_radio_get_my_id();
    }
}

void twr_radio_on_push_button(uint64_t *peer_device_address, uint16_t *event_count)
{
	(void) peer_device_address;
	(void) event_count;

    twr_led_pulse(&led, 100);
}

void application_init(void)
{
    // Initialize LED
    twr_led_init(&led, TWR_GPIO_LED, false, false);

    // Initialize button
    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, false);
    twr_button_set_event_handler(&button, button_event_handler, NULL);

    twr_button_init(&button_5s, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, false);
    twr_button_set_event_handler(&button_5s, button_5s_event_handler, NULL);
    twr_button_set_hold_time(&button_5s, 5000);

    // Initialize radio
    twr_radio_init(TWR_RADIO_MODE_GATEWAY);
    twr_radio_set_event_handler(radio_event_handler, NULL);
}
