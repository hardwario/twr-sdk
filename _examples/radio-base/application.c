#include <application.h>

// LED instance
hio_led_t led;

uint64_t my_did;
uint64_t peer_id;

// Button instance
hio_button_t button;
hio_button_t button_5s;

void button_event_handler(hio_button_t *self, hio_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == HIO_BUTTON_EVENT_HOLD)
    {
        hio_radio_pairing_mode_start();

        hio_led_set_mode(&led, HIO_LED_MODE_BLINK_FAST);
    }
}

static void button_5s_event_handler(hio_button_t *self, hio_button_event_t event, void *event_param)
{
	(void) self;
	(void) event_param;

	if (event == HIO_BUTTON_EVENT_HOLD)
	{
		hio_radio_pairing_mode_stop();

		uint64_t devices_address[HIO_RADIO_MAX_DEVICES];

		// Read all remote address
		hio_radio_get_peer_id(devices_address, HIO_RADIO_MAX_DEVICES);

		for (int i = 0; i < HIO_RADIO_MAX_DEVICES; i++)
		{
			if (devices_address[i] != 0)
			{
				// Remove device
			    hio_radio_peer_device_remove(devices_address[i]);
			}
		}

		hio_led_pulse(&led, 2000);
	}
}

void radio_event_handler(hio_radio_event_t event, void *event_param)
{
    (void) event_param;

    hio_led_set_mode(&led, HIO_LED_MODE_OFF);

    peer_id = hio_radio_get_event_id();

    if (event == HIO_RADIO_EVENT_ATTACH)
    {
        hio_led_pulse(&led, 1000);
    }
    else if (event == HIO_RADIO_EVENT_DETACH)
	{
		hio_led_pulse(&led, 3000);
	}
    else if (event == HIO_RADIO_EVENT_ATTACH_FAILURE)
    {
        hio_led_pulse(&led, 10000);
    }

    else if (event == HIO_RADIO_EVENT_INIT_DONE)
    {
        my_did = hio_radio_get_my_id();
    }
}

void hio_radio_on_push_button(uint64_t *peer_device_address, uint16_t *event_count)
{
	(void) peer_device_address;
	(void) event_count;

    hio_led_pulse(&led, 100);
}

void application_init(void)
{
    // Initialize LED
    hio_led_init(&led, HIO_GPIO_LED, false, false);

    // Initialize button
    hio_button_init(&button, HIO_GPIO_BUTTON, HIO_GPIO_PULL_DOWN, false);
    hio_button_set_event_handler(&button, button_event_handler, NULL);

    hio_button_init(&button_5s, HIO_GPIO_BUTTON, HIO_GPIO_PULL_DOWN, false);
    hio_button_set_event_handler(&button_5s, button_5s_event_handler, NULL);
    hio_button_set_hold_time(&button_5s, 5000);

    // Initialize radio
    hio_radio_init(HIO_RADIO_MODE_GATEWAY);
    hio_radio_set_event_handler(radio_event_handler, NULL);
}
