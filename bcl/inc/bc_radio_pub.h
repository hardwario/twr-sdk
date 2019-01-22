#ifndef _BC_RADIO_PUB_H
#define _BC_RADIO_PUB_H

#include <bc_radio.h>

//! @addtogroup bc_radio bc_radio
//! @brief Radio implementation send to gateway
//! @{

enum
{
    //! @brief channel 0:0
    BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT   = 0x00,
    //! @brief channel 0:1
    BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_ALTERNATE = 0x01,
    //! @brief channel 1:0
    BC_RADIO_PUB_CHANNEL_R1_I2C1_ADDRESS_DEFAULT   = 0x80,
    //! @brief channel 1:1
    BC_RADIO_PUB_CHANNEL_R1_I2C1_ADDRESS_ALTERNATE = 0x81,
    //! @brief channel 0:2
    BC_RADIO_PUB_CHANNEL_R2_I2C0_ADDRESS_DEFAULT   = 0x02,
    //! @brief channel 0:3
    BC_RADIO_PUB_CHANNEL_R2_I2C0_ADDRESS_ALTERNATE = 0x03,
    //! @brief channel 1:2
    BC_RADIO_PUB_CHANNEL_R2_I2C1_ADDRESS_DEFAULT   = 0x82,
    //! @brief channel 1:3
    BC_RADIO_PUB_CHANNEL_R2_I2C1_ADDRESS_ALTERNATE = 0x83,
    //! @brief channel 0:4
    BC_RADIO_PUB_CHANNEL_R3_I2C0_ADDRESS_DEFAULT   = 0x04,
    //! @brief channel 0:5
    BC_RADIO_PUB_CHANNEL_R3_I2C0_ADDRESS_ALTERNATE = 0x05,
    //! @brief channel 1:4
    BC_RADIO_PUB_CHANNEL_R3_I2C1_ADDRESS_DEFAULT   = 0x84,
    //! @brief channel 1:5
    BC_RADIO_PUB_CHANNEL_R3_I2C1_ADDRESS_ALTERNATE = 0x85,
    //! @brief channel a
    BC_RADIO_PUB_CHANNEL_A                         = 0xf0,
    //! @brief channel b
    BC_RADIO_PUB_CHANNEL_B                         = 0xf1,
    //! @brief channel set-point
    BC_RADIO_PUB_CHANNEL_SET_POINT                 = 0xf2
};

enum
{
    BC_RADIO_PUB_EVENT_PUSH_BUTTON = 0,
    BC_RADIO_PUB_EVENT_PIR_MOTION = 1,
    BC_RADIO_PUB_EVENT_LCD_BUTTON_LEFT = 2,
    BC_RADIO_PUB_EVENT_LCD_BUTTON_RIGHT = 3,
    BC_RADIO_PUB_EVENT_ACCELEROMETER_ALERT = 4,
    BC_RADIO_PUB_EVENT_HOLD_BUTTON = 5
};

enum
{
    BC_RADIO_PUB_STATE_LED = 0,
    BC_RADIO_PUB_STATE_RELAY_MODULE_0 = 1,
    BC_RADIO_PUB_STATE_RELAY_MODULE_1 = 2,
    BC_RADIO_PUB_STATE_POWER_MODULE_RELAY = 3
};

enum
{
    BC_RADIO_PUB_VALUE_HOLD_DURATION_BUTTON = 0
};

//! @brief Publish event count
//! @param[in] event_id Event id is from enum BC_RADIO_PUB_EVENT_*
//! @param[in] event_count Pointer to value, can be null
//! @return true On success
//! @return false On failure

bool bc_radio_pub_event_count(uint8_t event_id, uint16_t *event_count);

//! @brief Publish push button event count, same as use bc_radio_pub_event_count with BC_RADIO_PUB_EVENT_PUSH_BUTTON
//! @param[in] event_count Pointer to value, can be null
//! @return true On success
//! @return false On failure

bool bc_radio_pub_push_button(uint16_t *event_count);

//! @brief Publish temperature
//! @param[in] channel Channel id from enum BC_RADIO_PUB_CHANNEL_*
//! @param[in] celsius Pointer to value, can be null
//! @return true On success
//! @return false On failure

bool bc_radio_pub_temperature(uint8_t channel, float *celsius);

//! @brief Publish humidity
//! @param[in] channel Channel id from enum BC_RADIO_PUB_CHANNEL_*
//! @param[in] percentage Pointer to value, can be null
//! @return true On success
//! @return false On failure

bool bc_radio_pub_humidity(uint8_t channel, float *percentage);

//! @brief Publish luminosity
//! @param[in] channel Channel id from enum BC_RADIO_PUB_CHANNEL_*
//! @param[in] lux Pointer to value, can be null
//! @return true On success
//! @return false On failure

bool bc_radio_pub_luminosity(uint8_t channel, float *lux);

//! @brief Publish barometer
//! @param[in] channel Channel id from enum BC_RADIO_PUB_CHANNEL_*
//! @param[in] pascal Pointer to value, can be null
//! @param[in] meter Pointer to value, can be null
//! @return true On success
//! @return false On failure

bool bc_radio_pub_barometer(uint8_t channel, float *pascal, float *meter);

//! @brief Publish co2
//! @param[in] concentration Pointer to value, can be null
//! @return true On success
//! @return false On failure

bool bc_radio_pub_co2(float *concentration);

//! @brief Publish battery
//! @param[in] voltage Pointer to value, can be null
//! @return true On success
//! @return false On failure

bool bc_radio_pub_battery(float *voltage);

//! @brief Publish acceleration
//! @param[in] x_axis Pointer to value, can be null
//! @param[in] y_axis Pointer to value, can be null
//! @param[in] x_az_axisxis Pointer to value, can be null
//! @return true On success
//! @return false On failure

bool bc_radio_pub_acceleration(float *x_axis, float *y_axis, float *z_axis);

//! @brief Publish buffer
//! @param[in] buffer Pointer to buffer from which data will be send
//! @param[in] length Number of bytes to be send, max value is in BC_RADIO_NODE_MAX_BUFFER_SIZE
//! @return true On success
//! @return false On failure

bool bc_radio_pub_buffer(void *buffer, size_t length);

//! @brief Publish battery
//! @param[in] state_id State id from enum BC_RADIO_PUB_STATE_*
//! @param[in] state Pointer to value, can be null
//! @return true On success
//! @return false On failure

bool bc_radio_pub_state(uint8_t state_id, bool *state);

//! @brief Publish int value
//! @param[in] value_id State id from enum BC_RADIO_PUB_VALUE_*
//! @param[in] value Pointer to value, can be null
//! @return true On success

bool bc_radio_pub_value_int(uint8_t value_id, int *value);

//! @brief Publish bool value in custom topic
//! @param[in] subtopic Subtopic (example: node/{id}/{subtopic})
//! @param[in] value Pointer to value, can be null
//! @return true On success
//! @return false On failure

bool bc_radio_pub_bool(const char *subtopic, bool *value);

//! @brief Publish int value in custom topic
//! @param[in] subtopic Subtopic (example: node/{id}/{subtopic})
//! @param[in] value Pointer to value, can be null
//! @return true On success
//! @return false On failure

bool bc_radio_pub_int(const char *subtopic, int *value);

//! @brief Publish uint32 value in custom topic
//! @param[in] subtopic Subtopic (example: node/{id}/{subtopic})
//! @param[in] value Pointer to value, can be null
//! @return true On success
//! @return false On failure

bool bc_radio_pub_uint32(const char *subtopic, uint32_t *value);

//! @brief Publish float value in custom topic
//! @param[in] subtopic Subtopic (example: node/{id}/{subtopic})
//! @param[in] value Pointer to value, can be null
//! @return true On success
//! @return false On failure

bool bc_radio_pub_float(const char *subtopic, float *value);

//! @brief Publish string value in custom topic
//! @param[in] subtopic Subtopic (example: node/{id}/{subtopic})
//! @param[in] value Pointer to value, can be null
//! @return true On success
//! @return false On failure

bool bc_radio_pub_string(const char *subtopic, const char *value);

//! @brief Internal decode function for bc_radio.c
//! @param[in] id Pointer on sender id
//! @param[in] buffer Pointer to RX buffer
//! @param[in] length RX buffer length

void bc_radio_pub_decode(uint64_t *id, uint8_t *buffer, size_t length);

//! @}

#endif // _BC_RADIO_PUB_H
