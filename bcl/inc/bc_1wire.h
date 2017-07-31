#ifndef _BC_1WIRE_H
#define _BC_1WIRE_H

#include <bc_gpio.h>

//! @addtogroup bc_onewire bc_onewire
//! @brief Driver for 1-Wire
//! @{

#define BC_1WIRE_NO_DEVICE_NUMBER 0

//! @brief Initialize 1-Wire
//! @param channel GPIO channel

void bc_1wire_init(bc_gpio_channel_t channel);

//! @brief Reset the 1-Wire bus and return the presence of any device
//! @param channel GPIO channel
//! @return true Device present
//! @return false No device present

bool bc_1wire_reset(bc_gpio_channel_t channel);

//! @brief Select device
//! @param channel GPIO channel
//! @param[in] device_number Device number (for 0 skip ROM)

void bc_1wire_select(bc_gpio_channel_t channel, uint64_t *device_number);

//! @brief Select device
//! @param channel GPIO channel
//! @param[in] data Input data to be written
//! @param[in] length Number of bytes to be written

void bc_1wire_write(bc_gpio_channel_t channel, const void *buffer, size_t length);

//! @brief Select device
//! @param channel GPIO channel
//! @param[out] data Output which have been read
//! @param[in] length Number of bytes to be read

void bc_1wire_read(bc_gpio_channel_t channel, void *buffer, size_t length);

//! @brief Select device
//! @param channel GPIO channel
//! @param[in] data Input data to be written

void bc_1wire_write_8b(bc_gpio_channel_t channel, uint8_t data);

//! @brief Select device
//! @param channel GPIO channel
//! @return data which have been read

uint8_t bc_1wire_read_8b(bc_gpio_channel_t channel);

//! @brief Select device
//! @param channel GPIO channel
//! @param[in] data Input bit to be written

void bc_1wire_write_bit(bc_gpio_channel_t channel, uint8_t data);

//! @brief Select device
//! @param channel GPIO channel
//! @return bit which have been read

uint8_t bc_1wire_read_bit(bc_gpio_channel_t channel);

//! @brief Search for all devices on 1-Wire
//! @param[in] channel GPIO channel
//! @param[out] device_numbers List of device numbers
//! @param[in] length Number of bytes: list of device numbers
//! @return Number of found devices

int bc_1wire_search(bc_gpio_channel_t channel, uint64_t *device_numbers, size_t length);

//! @brief Search for all devices on 1-Wire with family code
//! @param[in] channel GPIO channel
//! @param[in] family_code
//! @param[out] device_numbers List of device numbers
//! @param[in] length Number of bytes: list of device numbers
//! @return Number of found devices

int bc_1wire_search_target(bc_gpio_channel_t channel, uint8_t family_code, uint64_t *device_numbers, size_t length);

//! @brief Calculate crc8
//! @param[in] buffer
//! @param[in] length Number of bytes
//! @param[in] The crc starting value
//! @return crc

uint8_t bc_1wire_crc8(const void *buffer, size_t length, uint8_t crc);

//! @brief Calculate crc16
//! @param[in] buffer
//! @param[in] length Number of bytes
//! @param[in] The crc starting value
//! @return crc

uint16_t bc_1wire_crc16(const void *buffer, size_t length, uint16_t crc);

//! @}

#endif /* _BC_1WIRE_H */
