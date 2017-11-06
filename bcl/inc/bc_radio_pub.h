#ifndef _BC_RADIO_PUB_H
#define _BC_RADIO_PUB_H

#include <bc_radio.h>

enum
{
    BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT   = 0x00,
    BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_ALTERNATE = 0x01,
    BC_RADIO_PUB_CHANNEL_R1_I2C1_ADDRESS_DEFAULT   = 0x80,
    BC_RADIO_PUB_CHANNEL_R1_I2C1_ADDRESS_ALTERNATE = 0x81,

    BC_RADIO_PUB_CHANNEL_R2_I2C0_ADDRESS_DEFAULT   = 0x02,
    BC_RADIO_PUB_CHANNEL_R2_I2C0_ADDRESS_ALTERNATE = 0x03,
    BC_RADIO_PUB_CHANNEL_R2_I2C1_ADDRESS_DEFAULT   = 0x82,
    BC_RADIO_PUB_CHANNEL_R2_I2C1_ADDRESS_ALTERNATE = 0x83,

    BC_RADIO_PUB_CHANNEL_R3_I2C0_ADDRESS_DEFAULT   = 0x04,
    BC_RADIO_PUB_CHANNEL_R3_I2C0_ADDRESS_ALTERNATE = 0x05,
    BC_RADIO_PUB_CHANNEL_R3_I2C1_ADDRESS_DEFAULT   = 0x84,
    BC_RADIO_PUB_CHANNEL_R3_I2C1_ADDRESS_ALTERNATE = 0x85,
};

enum
{
    BC_RADIO_PUB_EVENT_PUSH_BUTTON = 0,
    BC_RADIO_PUB_EVENT_PIR_MOTION = 1,
    BC_RADIO_PUB_EVENT_LCD_BUTTON_LEFT = 2,
    BC_RADIO_PUB_EVENT_LCD_BUTTON_RIGHT = 3,
    BC_RADIO_PUB_EVENT_ACCELEROMETER_ALERT = 4
};

enum
{
    BC_RADIO_PUB_STATE_LED = 0,
    BC_RADIO_PUB_STATE_RELAY_MODULE_0 = 1,
    BC_RADIO_PUB_STATE_RELAY_MODULE_1 = 2,
    BC_RADIO_PUB_STATE_POWER_MODULE_RELAY = 3,
};


bool bc_radio_pub_event_count(uint8_t event_id, uint16_t *event_count);

bool bc_radio_pub_push_button(uint16_t *event_count);

bool bc_radio_pub_thermometer(uint8_t channel, float *temperature);

bool bc_radio_pub_humidity(uint8_t channel, float *percentage);

bool bc_radio_pub_luminosity(uint8_t channel, float *lux);

bool bc_radio_pub_barometer(uint8_t channel, float *pascal, float *meter);

bool bc_radio_pub_co2(float *concentration);

bool bc_radio_pub_battery(float *voltage);

bool bc_radio_pub_buffer(void *buffer, size_t length);

bool bc_radio_pub_state(uint8_t state_id, bool *state);

bool bc_radio_node_state_set(uint64_t *id, uint8_t state_id, bool *state);

bool bc_radio_node_state_get(uint64_t *id, uint8_t state_id);

bool bc_radio_pub_bool(const char *subtopic, bool *value);

bool bc_radio_pub_int(const char *subtopic, int *value);

bool bc_radio_pub_float(const char *subtopic, float *value);

#endif // _BC_RADIO_PUB_H
