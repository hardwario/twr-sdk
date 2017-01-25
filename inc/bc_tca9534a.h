#ifndef _BC_TCA9534A_H
#define _BC_TCA9534A_H

#include <bc_common.h>
#include <bc_i2c.h>
#include <bc_tick.h>

#define BC_TCA9534A_DIRECTION_ALL_OUTPUT 0x00
#define BC_TCA9534A_DIRECTION_ALL_INPUT 0xFF

#define BC_TCA9534A_REGISTER_INPUT_PORT         0x00
#define BC_TCA9534A_REGISTER_OUTPUT_PORT        0x01
#define BC_TCA9534A_REGISTER_POLARITY_INVERSION 0x02
#define BC_TCA9534A_REGISTER_CONFIGURATION      0x03

typedef enum
{
    BC_TCA9534A_PIN_P0 = 0,
    BC_TCA9534A_PIN_P1 = 1,
    BC_TCA9534A_PIN_P2 = 2,
    BC_TCA9534A_PIN_P3 = 3,
    BC_TCA9534A_PIN_P4 = 4,
    BC_TCA9534A_PIN_P5 = 5,
    BC_TCA9534A_PIN_P6 = 6,
    BC_TCA9534A_PIN_P7 = 7

} bc_tca9534a_pin_t;

typedef enum
{
    BC_TCA9534A_DIRECTION_OUTPUT = 0,
    BC_TCA9534A_DIRECTION_INPUT = 1

} bc_tca9534a_direction_t;

typedef enum
{
    BC_TCA9534A_VALUE_LOW = 0,
    BC_TCA9534A_VALUE_HIGH = 1

} bc_tca9534a_value_t;

typedef struct
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
} bc_tca9534a_t;

bool bc_tca9534a_init(bc_tca9534a_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);
bool bc_tca9534a_read_port(bc_tca9534a_t *self, uint8_t *value);
bool bc_tca9534a_write_port(bc_tca9534a_t *self, uint8_t value);
bool bc_tca9534a_read_pin(bc_tca9534a_t *self, bc_tca9534a_pin_t pin, bc_tca9534a_value_t *value);
bool bc_tca9534a_write_pin(bc_tca9534a_t *self, bc_tca9534a_pin_t pin, bc_tca9534a_value_t value);
bool bc_tca9534a_get_port_direction(bc_tca9534a_t *self, uint8_t *direction);
bool bc_tca9534a_set_port_direction(bc_tca9534a_t *self, uint8_t direction);
bool bc_tca9534a_get_pin_direction(bc_tca9534a_t *self, bc_tca9534a_pin_t pin,
                                       bc_tca9534a_direction_t *direction);
bool bc_tca9534a_set_pin_direction(bc_tca9534a_t *self, bc_tca9534a_pin_t pin,
                                       bc_tca9534a_direction_t direction);


void bc_tca9534a_test(void);

#endif // _BC_TCA9534A_H
