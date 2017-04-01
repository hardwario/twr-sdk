#include <usb_talk.h>
#include <bc_scheduler.h>
#include <bc_usb_cdc.h>
#include <jsmn.h>
#include <base64.h>
// TODO
// #include "bc_module_power.h"

#define USB_TALK_TOKEN_ARRAY         0
#define USB_TALK_TOKEN_TOPIC         1
#define USB_TALK_TOKEN_PAYLOAD       2
#define USB_TALK_TOKEN_PAYLOAD_KEY   3
#define USB_TALK_TOKEN_PAYLOAD_VALUE 4

#define USB_TALK_UINT_VALUE_NULL -1
#define USB_TALK_UINT_VALUE_INVALID -2

usb_talk_t usb_talk;

static void _usb_talk_task(void *param);
static void usb_talk_process_character(char character);
static void usb_talk_process_message(char *message, size_t length);
static bool usb_talk_on_message_led_strip(const char *buffer, int token_count, jsmntok_t *tokens);
static bool usb_talk_on_message_led_strip_config(const char *buffer, int token_count, jsmntok_t *tokens);
static bool usb_talk_on_message_light_set(const char *buffer, int token_count, jsmntok_t *tokens);
static bool usb_talk_on_message_light_get(const char *buffer, int token_count, jsmntok_t *tokens);
static bool usb_talk_on_message_relay_set(const char *buffer, int token_count, jsmntok_t *tokens);
static bool usb_talk_on_message_relay_get(const char *buffer, int token_count, jsmntok_t *tokens);
static bool usb_talk_is_string_token_equal(const char *buffer, jsmntok_t *token, const char *value);
static int usb_talk_token_get_uint(const char *buffer, jsmntok_t *token);
static void usb_talk_send_string(const char *buffer);

void usb_talk_init(void)
{
    memset(&usb_talk, 0, sizeof(usb_talk));

    bc_usb_cdc_init();

    bc_scheduler_register(_usb_talk_task, NULL, 0);
}

void usb_talk_publish_push_button(const char *prefix, uint16_t *event_count)
{
    if (event_count != NULL)
    {
        snprintf(usb_talk.tx_buffer, sizeof(usb_talk.tx_buffer),
                 "[\"%spush-button/-\", {\"event-count\": %" PRIu16 "}]\n", prefix, *event_count);
    }
    else
    {
        snprintf(usb_talk.tx_buffer, sizeof(usb_talk.tx_buffer), "[\"%spush-button/-\", {\"event-count\": null}]\n",
                 prefix);
    }

    usb_talk_send_string((const char *) usb_talk.tx_buffer);
}

void usb_talk_publish_input_change(const char *prefix, uint16_t *value)
{

	snprintf(usb_talk.tx_buffer, sizeof(usb_talk.tx_buffer),
			 "[\"%sinput-change/-\", {\"value\": %" PRIu16 "}]\n", prefix, *value);


    usb_talk_send_string((const char *) usb_talk.tx_buffer);
}

void usb_talk_publish_light(void)
{
    snprintf(usb_talk.tx_buffer, sizeof(usb_talk.tx_buffer), "[\"base/light/-\", {\"state\": %s}]\n",
             usb_talk.light_is_on ? "true" : "false");

    usb_talk_send_string((const char *) usb_talk.tx_buffer);
}

void usb_talk_publish_relay(void)
{
    // TODO
    snprintf(usb_talk.tx_buffer, sizeof(usb_talk.tx_buffer), "[\"base/relay/-\", {\"state\": %s}]\n",
             /* bc_module_power.relay_is_on */ true ? "true" : "false");

    usb_talk_send_string((const char *) usb_talk.tx_buffer);
}

void usb_talk_publish_led_strip_config(const char *sufix)
{
    // TODO
    snprintf(usb_talk.tx_buffer, sizeof(usb_talk.tx_buffer), "[\"base/led-strip/-/config%s\", {\"mode\": \"%s\", \"count\": %d}]\n",
    		 sufix,
    		 /* bc_module_power.led_strip_mode == BC_MODULE_POWER_RGBW */ true ? "rgbw" : "rgb",
             /* bc_module_power.led_strip_count */ 150);

    usb_talk_send_string((const char *) usb_talk.tx_buffer);
}

void usb_talk_publish_thermometer(const char *prefix, float *temperature)
{
    if (temperature != NULL)
    {
        snprintf(usb_talk.tx_buffer, sizeof(usb_talk.tx_buffer),
                 "[\"%sthermometer/i2c0-49\", {\"temperature\": [%0.2f, \"\\u2103\"]}]\n", prefix, *temperature);
    }
    else
    {
        snprintf(usb_talk.tx_buffer, sizeof(usb_talk.tx_buffer),
                 "[\"%sthermometer/i2c0-49\", {\"temperature\": null}]\n", prefix);
    }

    usb_talk_send_string((const char *) usb_talk.tx_buffer);
}

void usb_talk_publish_humidity_sensor(const char *prefix, float *relative_humidity)
{
    if (relative_humidity != NULL)
    {
        snprintf(usb_talk.tx_buffer, sizeof(usb_talk.tx_buffer),
                 "[\"%shumidity-sensor/i2c0-40\", {\"relative-humidity\": [%0.1f, \"%%\"]}]\n", prefix,
                 *relative_humidity);
    }
    else
    {
        snprintf(usb_talk.tx_buffer, sizeof(usb_talk.tx_buffer),
                 "[\"%shumidity-sensor/i2c0-40\", {\"relative-humidity\": null}]\n", prefix);
    }

    usb_talk_send_string((const char *) usb_talk.tx_buffer);
}


void usb_talk_publish_ir_rx(const char *prefix, uint32_t *ir_code)
{
    if (ir_code != NULL)
    {
        snprintf(usb_talk.tx_buffer, sizeof(usb_talk.tx_buffer),
                 "[\"%sinfrared/i2c0-00\", {\"infrared-receive\": \"0x%08x\"}]\n", prefix,
                 (unsigned int)*ir_code);
    }
    else
    {
        snprintf(usb_talk.tx_buffer, sizeof(usb_talk.tx_buffer),
                 "[\"%sinfrared/i2c0-00\", {\"infrared-receive\": null}]\n", prefix);
    }

    usb_talk_send_string((const char *) usb_talk.tx_buffer);
}

static void _usb_talk_task(void *param)
{
    (void) param;

    while (true)
    {
        static uint8_t buffer[16];

        size_t length = bc_usb_cdc_read(buffer, sizeof(buffer));

        if (length == 0)
        {
            break;
        }

        for (size_t i = 0; i < length; i++)
        {
            usb_talk_process_character((char) buffer[i]);
        }
    }

    // TODO
    bc_scheduler_plan_current_now();
}

static void usb_talk_process_character(char character)
{
    if (character == '\n')
    {
        if (!usb_talk.rx_error && usb_talk.rx_length > 0)
        {
            usb_talk_process_message(usb_talk.rx_buffer, usb_talk.rx_length);
        }

        usb_talk.rx_length = 0;
        usb_talk.rx_error = false;

        return;
    }

    if (usb_talk.rx_length == sizeof(usb_talk.rx_buffer))
    {
        usb_talk.rx_error = true;
    }
    else
    {
        if (!usb_talk.rx_error)
        {
            usb_talk.rx_buffer[usb_talk.rx_length++] = character;
        }
    }
}

static void usb_talk_process_message(char *message, size_t length)
{
    static jsmn_parser parser;
    static jsmntok_t tokens[16];

    jsmn_init(&parser);

    int token_count = jsmn_parse(&parser, (const char *) message, length, tokens, 16);

    if (token_count < 3)
    {
        return;
    }

    if (tokens[USB_TALK_TOKEN_ARRAY].type != JSMN_ARRAY || tokens[USB_TALK_TOKEN_ARRAY].size != 2)
    {
        return;
    }

    if (tokens[USB_TALK_TOKEN_TOPIC].type != JSMN_STRING || tokens[USB_TALK_TOKEN_TOPIC].size != 0)
    {
        return;
    }

    if (tokens[USB_TALK_TOKEN_PAYLOAD].type != JSMN_OBJECT)
    {
        return;
    }

    if (usb_talk_on_message_led_strip((const char *) message, token_count, tokens))
    {
        return;
    }

    if (usb_talk_on_message_light_set((const char *) message, token_count, tokens))
    {
        return;
    }

    if (usb_talk_on_message_light_get((const char *) message, token_count, tokens))
    {
        return;
    }

    if (usb_talk_on_message_relay_set((const char *) message, token_count, tokens))
    {
        return;
    }

    if (usb_talk_on_message_relay_get((const char *) message, token_count, tokens))
    {
        return;
    }

    if (usb_talk_on_message_led_strip_config((const char *) message, token_count, tokens))
    {
        return;
    }
}

static bool usb_talk_on_message_led_strip(const char *buffer, int token_count, jsmntok_t *tokens)
{
    if (token_count != 5)
    {
        return false;
    }

    if (tokens[USB_TALK_TOKEN_PAYLOAD].size != 1)
    {
        return false;
    }

    if (tokens[USB_TALK_TOKEN_PAYLOAD_KEY].type != JSMN_STRING || tokens[USB_TALK_TOKEN_PAYLOAD_KEY].size != 1)
    {
        return false;
    }

    if (tokens[USB_TALK_TOKEN_PAYLOAD_VALUE].type != JSMN_STRING || tokens[USB_TALK_TOKEN_PAYLOAD_VALUE].size != 0)
    {
        return false;
    }

    // TODO
    int length = /* bc_module_power.led_strip_count */ 150 * /* bc_module_power.led_strip_mode */ 4;

    int base64_size = (length + 2 - ((length + 2) % 3)) * 4 / 3;

    if ((tokens[USB_TALK_TOKEN_PAYLOAD_VALUE].end - tokens[USB_TALK_TOKEN_PAYLOAD_VALUE].start) != base64_size)
    {
        return false;
    }

    if (!usb_talk_is_string_token_equal(buffer, &tokens[USB_TALK_TOKEN_TOPIC], "base/led-strip/-/set"))
    {
        return false;
    }

    if (!usb_talk_is_string_token_equal(buffer, &tokens[USB_TALK_TOKEN_PAYLOAD_KEY], "pixels"))
    {
        return false;
    }

    uint32_t output_length;

    if (!base64_decode(&buffer[tokens[USB_TALK_TOKEN_PAYLOAD_VALUE].start], base64_size, usb_talk.pixels, &output_length))
    {
        return false;
    }

    usb_talk_send_string("[\"base/led-strip/-/set/ok\", {}]\n");

    return true;
}

static bool usb_talk_on_message_led_strip_config(const char *buffer, int token_count, jsmntok_t *tokens)
{
    if ((token_count != 5) && (token_count != 7))
    {
        return false;
    }

    if (tokens[USB_TALK_TOKEN_PAYLOAD].size == 0)
    {
        return false;
    }

    if (!usb_talk_is_string_token_equal(buffer, &tokens[USB_TALK_TOKEN_TOPIC], "base/led-strip/-/config/set"))
    {
        return false;
    }

    int i;

    for (i = 3; i < tokens[USB_TALK_TOKEN_PAYLOAD].size * 2 + 3 && i + 1 < token_count; i += 2)
    {
        if (usb_talk_is_string_token_equal(buffer, &tokens[i], "mode"))
        {
            if (usb_talk_is_string_token_equal(buffer, &tokens[i + 1], "rgbw"))
            {
                // TODO
                // bc_module_power.led_strip_mode = BC_MODULE_POWER_RGBW;
            }
            else if (usb_talk_is_string_token_equal(buffer, &tokens[i + 1], "rgb"))
            {
                // TODO
                // bc_module_power.led_strip_mode = BC_MODULE_POWER_RGB;
            }
            else
			{
				return true;
			}
        }
        else if (usb_talk_is_string_token_equal(buffer, &tokens[i], "count"))
        {
            int count = usb_talk_token_get_uint(buffer, &tokens[i + 1]);

            // TODO
            if ((0 < count) && (count <= /* BC_MODULE_POWER_MAX_LED_STRIP_COUNT */ 150))
            {
                // TODO
                // bc_module_power.led_strip_count = count;
            }
            else
            {
            	return true;
            }
        }
        else
		{
			return true;
		}
    }

    usb_talk_publish_led_strip_config("/set/ok");

    return true;
}

static bool usb_talk_on_message_light_set(const char *buffer, int token_count, jsmntok_t *tokens)
{
    if (token_count != 5)
    {
        return false;
    }

    if (tokens[USB_TALK_TOKEN_PAYLOAD].size != 1)
    {
        return false;
    }

    if (tokens[USB_TALK_TOKEN_PAYLOAD_KEY].type != JSMN_STRING || tokens[USB_TALK_TOKEN_PAYLOAD_KEY].size != 1)
    {
        return false;
    }

    if (tokens[USB_TALK_TOKEN_PAYLOAD_VALUE].type != JSMN_PRIMITIVE || tokens[USB_TALK_TOKEN_PAYLOAD_VALUE].size != 0)
    {
        return false;
    }

    if (!usb_talk_is_string_token_equal(buffer, &tokens[USB_TALK_TOKEN_TOPIC], "base/light/-/set"))
    {
        return false;
    }

    if (!usb_talk_is_string_token_equal(buffer, &tokens[USB_TALK_TOKEN_PAYLOAD_KEY], "state"))
    {
        return false;
    }

    if (usb_talk_is_string_token_equal(buffer, &tokens[USB_TALK_TOKEN_PAYLOAD_VALUE], "true"))
    {
        usb_talk.light_is_on = true;
    }
    else if (usb_talk_is_string_token_equal(buffer, &tokens[USB_TALK_TOKEN_PAYLOAD_VALUE], "false"))
    {
        usb_talk.light_is_on = false;
    }

    usb_talk_publish_light();

    return true;
}

static bool usb_talk_on_message_light_get(const char *buffer, int token_count, jsmntok_t *tokens)
{
    if (token_count != 3)
    {
        return false;
    }

    if (tokens[USB_TALK_TOKEN_PAYLOAD].size != 0)
    {
        return false;
    }

    if (!usb_talk_is_string_token_equal(buffer, &tokens[USB_TALK_TOKEN_TOPIC], "base/light/-/get"))
    {
        return false;
    }

    usb_talk_publish_light();

    return true;
}

static bool usb_talk_on_message_relay_set(const char *buffer, int token_count, jsmntok_t *tokens)
{
    if (token_count != 5)
    {
        return false;
    }

    if (tokens[USB_TALK_TOKEN_PAYLOAD].size != 1)
    {
        return false;
    }

    if (tokens[USB_TALK_TOKEN_PAYLOAD_KEY].type != JSMN_STRING || tokens[USB_TALK_TOKEN_PAYLOAD_KEY].size != 1)
    {
        return false;
    }

    if (tokens[USB_TALK_TOKEN_PAYLOAD_VALUE].type != JSMN_PRIMITIVE || tokens[USB_TALK_TOKEN_PAYLOAD_VALUE].size != 0)
    {
        return false;
    }

    if (!usb_talk_is_string_token_equal(buffer, &tokens[USB_TALK_TOKEN_TOPIC], "base/relay/-/set"))
    {
        return false;
    }

    if (!usb_talk_is_string_token_equal(buffer, &tokens[USB_TALK_TOKEN_PAYLOAD_KEY], "state"))
    {
        return false;
    }

    if (usb_talk_is_string_token_equal(buffer, &tokens[USB_TALK_TOKEN_PAYLOAD_VALUE], "true"))
    {
        // TODO
        // bc_module_power.relay_is_on = true;
    }
    else if (usb_talk_is_string_token_equal(buffer, &tokens[USB_TALK_TOKEN_PAYLOAD_VALUE], "false"))
    {
        // TODO
        // bc_module_power.relay_is_on = false;
    }

    usb_talk_publish_relay();

    return true;
}

static bool usb_talk_on_message_relay_get(const char *buffer, int token_count, jsmntok_t *tokens)
{
    if (token_count != 3)
    {
        return false;
    }

    if (tokens[USB_TALK_TOKEN_PAYLOAD].size != 0)
    {
        return false;
    }

    if (!usb_talk_is_string_token_equal(buffer, &tokens[USB_TALK_TOKEN_TOPIC], "base/relay/-/get"))
    {
        return false;
    }

    usb_talk_publish_relay();

    return true;
}

static bool usb_talk_is_string_token_equal(const char *buffer, jsmntok_t *token, const char *value)
{
    size_t token_length;

    token_length = (size_t) (token->end - token->start);

    if (strlen(value) != token_length)
    {
        return false;
    }

    if (strncmp(value, &buffer[token->start], token_length) != 0)
    {
        return false;
    }

    return true;
}

// TODO I think there is possibly buffer overflow and also will not work for uint32 since we are storing to int and then comparing to < 0

static int usb_talk_token_get_uint(const char *buffer, jsmntok_t *token)
{
    if (token->type != JSMN_PRIMITIVE)
    {
        return USB_TALK_UINT_VALUE_INVALID;
    }

    size_t length = (size_t) (token->end - token->start);

    char str[10 + 1]; // TODO I have added extra byte for \0, 10 characters will hold 32-bit number

    if (length > sizeof(str))
    {
        return USB_TALK_UINT_VALUE_INVALID;
    }

    memset(str, 0, sizeof(str));

    strncpy(str, buffer + token->start, length);

    if (strcmp(str, "null") == 0)
    {
        return USB_TALK_UINT_VALUE_NULL;
    }

    int ret;

    if (strchr(str, 'e'))
    {
        ret = (int) strtof(str, NULL);
    }
    else
    {
        ret = (int) strtol(str, NULL, 10);
    }

    if (ret < 0)
    {
        return USB_TALK_UINT_VALUE_INVALID;
    }

    return ret;
}

static void usb_talk_send_string(const char *buffer)
{
    bc_usb_cdc_write(buffer, strlen(buffer));
}
