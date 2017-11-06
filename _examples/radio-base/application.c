#include <application.h>

// LED instance
bc_led_t led;

uint64_t my_did;
uint64_t peer_id;

// Button instance
bc_button_t button;
bc_button_t button_5s;

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_HOLD)
    {
        bc_radio_pairing_mode_start();

        bc_led_set_mode(&led, BC_LED_MODE_BLINK_FAST);
    }
}

static void button_5s_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
	(void) self;
	(void) event_param;

	if (event == BC_BUTTON_EVENT_HOLD)
	{
		bc_radio_pairing_mode_stop();

		uint64_t devices_address[BC_RADIO_MAX_DEVICES];

		// Read all remote address
		bc_radio_get_peer_id(devices_address, BC_RADIO_MAX_DEVICES);

		for (int i = 0; i < BC_RADIO_MAX_DEVICES; i++)
		{
			if (devices_address[i] != 0)
			{
				// Remove device
			    bc_radio_peer_device_remove(devices_address[i]);
			}
		}

		bc_led_pulse(&led, 2000);
	}
}

void radio_event_handler(bc_radio_event_t event, void *event_param)
{
    (void) event_param;

    bc_led_set_mode(&led, BC_LED_MODE_OFF);

    peer_id = bc_radio_get_event_id();

    if (event == BC_RADIO_EVENT_ATTACH)
    {
        bc_led_pulse(&led, 1000);
    }
    else if (event == BC_RADIO_EVENT_DETACH)
	{
		bc_led_pulse(&led, 3000);
	}
    else if (event == BC_RADIO_EVENT_ATTACH_FAILURE)
    {
        bc_led_pulse(&led, 10000);
    }

    else if (event == BC_RADIO_EVENT_INIT_DONE)
    {
        my_did = bc_radio_get_my_id();
    }
}

void bc_radio_on_push_button(uint64_t *peer_device_address, uint16_t *event_count)
{
	(void) peer_device_address;
	(void) event_count;

    bc_led_pulse(&led, 100);
}

void application_init(void)
{
    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);

    // Initialize button
    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    bc_button_init(&button_5s, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button_5s, button_5s_event_handler, NULL);
    bc_button_set_hold_time(&button_5s, 5000);

    // Initialize radio
    bc_radio_init(BC_RADIO_MODE_GATEWAY);
    bc_radio_set_event_handler(radio_event_handler, NULL);
}
