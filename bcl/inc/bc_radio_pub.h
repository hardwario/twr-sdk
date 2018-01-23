#ifndef _BC_RADIO_PUB_H
#define _BC_RADIO_PUB_H

#include <bc_radio.h>

//! @addtogroup bc_radio bc_radio
//! @brief Radio implementation send to gateway
//! @{

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

    BC_RADIO_PUB_CHANNEL_A                         = 0xf0,
    BC_RADIO_PUB_CHANNEL_B                         = 0xf1,
    BC_RADIO_PUB_CHANNEL_SET_POINT                 = 0xf2
};

enum
{
    BC_RADIO_PUB_EVENT_PUSH_BUTTON = 0,
    BC_RADIO_PUB_EVENT_PIR_MOTION = 1,
    BC_RADIO_PUB_EVENT_LCD_BUTTON_LEFT = 2,
    BC_RADIO_PUB_EVENT_LCD_BUTTON_RIGHT = 3,
    BC_RADIO_PUB_EVENT_ACCELEROMETER_ALERT = 4,
    BC_RADIO_PUB_EVENT_ENCODER_INCREMENT = 5
};

enum
{
    BC_RADIO_PUB_STATE_LED = 0,
    BC_RADIO_PUB_STATE_RELAY_MODULE_0 = 1,
    BC_RADIO_PUB_STATE_RELAY_MODULE_1 = 2,
    BC_RADIO_PUB_STATE_POWER_MODULE_RELAY = 3
};


bool bc_radio_pub_event_count(uint8_t event_id, uint16_t *event_count);

bool bc_radio_pub_push_button(uint16_t *event_count);

bool bc_radio_pub_temperature(uint8_t channel, float *celsius);

bool bc_radio_pub_humidity(uint8_t channel, float *percentage);

bool bc_radio_pub_luminosity(uint8_t channel, float *lux);

bool bc_radio_pub_barometer(uint8_t channel, float *pascal, float *meter);

bool bc_radio_pub_co2(float *concentration);

bool bc_radio_pub_battery(float *voltage);

bool bc_radio_pub_buffer(void *buffer, size_t length);

bool bc_radio_pub_state(uint8_t state_id, bool *state);

bool bc_radio_pub_bool(const char *subtopic, bool *value);

bool bc_radio_pub_int(const char *subtopic, int *value);

bool bc_radio_pub_uint32(const char *subtopic, uint32_t *value);

bool bc_radio_pub_float(const char *subtopic, float *value);

//! @brief Internal decode function for bc_radio.c
//! @param[in] id Pointer on sender id
//! @param[in] buffer Pointer to RX buffer
//! @param[in] length RX buffer length

void bc_radio_pub_decode(uint64_t *id, uint8_t *buffer, size_t length);

//! @}

#endif // _BC_RADIO_PUB_H
