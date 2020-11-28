#ifndef _HIO_TCA9534A_H
#define _HIO_TCA9534A_H

#include <hio_i2c.h>
#include <hio_tick.h>

#define HIO_TCA9534A_PIN_STATE_LOW   0
#define HIO_TCA9534A_PIN_STATE_HIGH  1

//! @addtogroup hio_tca9534a hio_tca9534a
//! @brief Driver for TCA9534A I/O expander
//! @{

//! @brief Individual pin names

typedef enum
{
    HIO_TCA9534A_PIN_P0 = 0,
    HIO_TCA9534A_PIN_P1 = 1,
    HIO_TCA9534A_PIN_P2 = 2,
    HIO_TCA9534A_PIN_P3 = 3,
    HIO_TCA9534A_PIN_P4 = 4,
    HIO_TCA9534A_PIN_P5 = 5,
    HIO_TCA9534A_PIN_P6 = 6,
    HIO_TCA9534A_PIN_P7 = 7

} hio_tca9534a_pin_t;

//! @brief Pin direction

typedef enum
{
    HIO_TCA9534A_PIN_DIRECTION_OUTPUT = 0,
    HIO_TCA9534A_PIN_DIRECTION_INPUT = 1

} hio_tca9534a_pin_direction_t;

//! @brief Pin state

//! @brief TCA9534A instance

typedef struct
{
    hio_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    uint8_t _direction;
    uint8_t _output_port;

} hio_tca9534a_t;

//! @brief Initialize TCA9534A
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

bool hio_tca9534a_init(hio_tca9534a_t *self, hio_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Read state of all pins
//! @param[in] self Instance
//! @param[out] state Pointer to variable where state of all pins will be stored
//! @return true On success
//! @return false On failure

bool hio_tca9534a_read_port(hio_tca9534a_t *self, uint8_t *state);

//! @brief Write state to all pins
//! @param[in] self Instance
//! @param[in] state Desired state of all pins
//! @return true On success
//! @return false On failure

bool hio_tca9534a_write_port(hio_tca9534a_t *self, uint8_t state);

//! @brief Read pin state
//! @param[in] self Instance
//! @param[in] pin Pin name
//! @param[out] state Pointer to variable where state of pin will be stored
//! @return true On success
//! @return false On failure

bool hio_tca9534a_read_pin(hio_tca9534a_t *self, hio_tca9534a_pin_t pin, int *state);

//! @brief Write pin state
//! @param[in] self Instance
//! @param[in] pin Pin name
//! @param[in] state Desired state of pin
//! @return true On success
//! @return false On failure

bool hio_tca9534a_write_pin(hio_tca9534a_t *self, hio_tca9534a_pin_t pin, int state);

//! @brief Get direction of all pins
//! @param[in] self Instance
//! @param[out] direction Pointer to variable where direction of all pins will be stored
//! @return true On success
//! @return false On failure

bool hio_tca9534a_get_port_direction(hio_tca9534a_t *self, uint8_t *direction);

//! @brief Set direction of all pins
//! @param[in] self Instance
//! @param[in] direction Desired direction of all pins
//! @return true On success
//! @return false On failure

bool hio_tca9534a_set_port_direction(hio_tca9534a_t *self, uint8_t direction);

//! @brief Get pin direction
//! @param[in] self Instance
//! @param[in] pin Pin name
//! @param[out] direction Pointer to variable where direction of pin will be stored
//! @return true On success
//! @return false On failure

bool hio_tca9534a_get_pin_direction(hio_tca9534a_t *self, hio_tca9534a_pin_t pin, hio_tca9534a_pin_direction_t *direction);

//! @brief Set pin direction
//! @param[in] self Instance
//! @param[in] pin Pin name
//! @param[in] direction Desired pin direction
//! @return true On success
//! @return false On failure

bool hio_tca9534a_set_pin_direction(hio_tca9534a_t *self, hio_tca9534a_pin_t pin, hio_tca9534a_pin_direction_t direction);

//! @}

#endif // _HIO_TCA9534A_H
