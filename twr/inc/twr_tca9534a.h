#ifndef _TWR_TCA9534A_H
#define _TWR_TCA9534A_H

#include <twr_i2c.h>
#include <twr_tick.h>

#define TWR_TCA9534A_PIN_STATE_LOW   0
#define TWR_TCA9534A_PIN_STATE_HIGH  1

//! @addtogroup twr_tca9534a twr_tca9534a
//! @brief Driver for TCA9534A I/O expander
//! @{

//! @brief Individual pin names

typedef enum
{
    TWR_TCA9534A_PIN_P0 = 0,
    TWR_TCA9534A_PIN_P1 = 1,
    TWR_TCA9534A_PIN_P2 = 2,
    TWR_TCA9534A_PIN_P3 = 3,
    TWR_TCA9534A_PIN_P4 = 4,
    TWR_TCA9534A_PIN_P5 = 5,
    TWR_TCA9534A_PIN_P6 = 6,
    TWR_TCA9534A_PIN_P7 = 7

} twr_tca9534a_pin_t;

//! @brief Pin direction

typedef enum
{
    TWR_TCA9534A_PIN_DIRECTION_OUTPUT = 0,
    TWR_TCA9534A_PIN_DIRECTION_INPUT = 1

} twr_tca9534a_pin_direction_t;

//! @brief Pin state

//! @brief TCA9534A instance

typedef struct
{
    twr_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    uint8_t _direction;
    uint8_t _output_port;

} twr_tca9534a_t;

//! @brief Initialize TCA9534A
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

bool twr_tca9534a_init(twr_tca9534a_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Read state of all pins
//! @param[in] self Instance
//! @param[out] state Pointer to variable where state of all pins will be stored
//! @return true On success
//! @return false On failure

bool twr_tca9534a_read_port(twr_tca9534a_t *self, uint8_t *state);

//! @brief Write state to all pins
//! @param[in] self Instance
//! @param[in] state Desired state of all pins
//! @return true On success
//! @return false On failure

bool twr_tca9534a_write_port(twr_tca9534a_t *self, uint8_t state);

//! @brief Read pin state
//! @param[in] self Instance
//! @param[in] pin Pin name
//! @param[out] state Pointer to variable where state of pin will be stored
//! @return true On success
//! @return false On failure

bool twr_tca9534a_read_pin(twr_tca9534a_t *self, twr_tca9534a_pin_t pin, int *state);

//! @brief Write pin state
//! @param[in] self Instance
//! @param[in] pin Pin name
//! @param[in] state Desired state of pin
//! @return true On success
//! @return false On failure

bool twr_tca9534a_write_pin(twr_tca9534a_t *self, twr_tca9534a_pin_t pin, int state);

//! @brief Get direction of all pins
//! @param[in] self Instance
//! @param[out] direction Pointer to variable where direction of all pins will be stored
//! @return true On success
//! @return false On failure

bool twr_tca9534a_get_port_direction(twr_tca9534a_t *self, uint8_t *direction);

//! @brief Set direction of all pins
//! @param[in] self Instance
//! @param[in] direction Desired direction of all pins
//! @return true On success
//! @return false On failure

bool twr_tca9534a_set_port_direction(twr_tca9534a_t *self, uint8_t direction);

//! @brief Get pin direction
//! @param[in] self Instance
//! @param[in] pin Pin name
//! @param[out] direction Pointer to variable where direction of pin will be stored
//! @return true On success
//! @return false On failure

bool twr_tca9534a_get_pin_direction(twr_tca9534a_t *self, twr_tca9534a_pin_t pin, twr_tca9534a_pin_direction_t *direction);

//! @brief Set pin direction
//! @param[in] self Instance
//! @param[in] pin Pin name
//! @param[in] direction Desired pin direction
//! @return true On success
//! @return false On failure

bool twr_tca9534a_set_pin_direction(twr_tca9534a_t *self, twr_tca9534a_pin_t pin, twr_tca9534a_pin_direction_t direction);

//! @}

#endif // _TWR_TCA9534A_H
