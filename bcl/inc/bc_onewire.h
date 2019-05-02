#ifndef _BC_ONEWIRE_H
#define _BC_ONEWIRE_H

#include <bc_gpio.h>

//! @addtogroup bc_onewire bc_onewire
//! @brief Driver for 1-Wire
//! @{

#define BC_ONEWIRE_DEVICE_NUMBER_SKIP_ROM 0

//! @brief Initialize 1-Wire
//! @param channel GPIO channel

void bc_onewire_init(bc_gpio_channel_t channel);

//! @brief Start transaction, enable pll and run timer
//! @param channel GPIO channel
//! @return true On success
//! @return false On failure

bool bc_onewire_transaction_start(bc_gpio_channel_t channel);

//! @brief Stop transaction
//! @param channel GPIO channel
//! @return true On success
//! @return false On failure

bool bc_onewire_transaction_stop(bc_gpio_channel_t channel);

//! @brief Reset the 1-Wire bus and return the presence of any device
//! @param channel GPIO channel
//! @return true Device present
//! @return false No device present

bool bc_onewire_reset(bc_gpio_channel_t channel);

//! @brief Select device
//! @param channel GPIO channel
//! @param[in] device_number Device number (for 0 skip ROM)

void bc_onewire_select(bc_gpio_channel_t channel, uint64_t *device_number);

//! @brief Skip ROM
//! @param channel GPIO channel

void bc_onewire_skip_rom(bc_gpio_channel_t channel);

//! @brief Select device
//! @param channel GPIO channel
//! @param[in] data Input data to be written
//! @param[in] length Number of bytes to be written

void bc_onewire_write(bc_gpio_channel_t channel, const void *buffer, size_t length);

//! @brief Select device
//! @param channel GPIO channel
//! @param[out] data Output which have been read
//! @param[in] length Number of bytes to be read

void bc_onewire_read(bc_gpio_channel_t channel, void *buffer, size_t length);

//! @brief Select device
//! @param channel GPIO channel
//! @param[in] data Input data to be written

void bc_onewire_write_8b(bc_gpio_channel_t channel, uint8_t data);

//! @brief Select device
//! @param channel GPIO channel
//! @return data which have been read

uint8_t bc_onewire_read_8b(bc_gpio_channel_t channel);

//! @brief Select device
//! @param channel GPIO channel
//! @param[in] bit Input bit to be written

void bc_onewire_write_bit(bc_gpio_channel_t channel, int bit);

//! @brief Select device
//! @param channel GPIO channel
//! @return bit which have been read

int bc_onewire_read_bit(bc_gpio_channel_t channel);

//! @brief Search for all devices on 1-Wire
//! @param[in] channel GPIO channel
//! @param[out] device_list Pointer to destination array holding list of devices
//! @param[in] device_list_size Size of array holding list of devices
//! @return Number of found devices

int bc_onewire_search_all(bc_gpio_channel_t channel, uint64_t *device_list, size_t device_list_size);

//! @brief Search for all devices on 1-Wire with family code
//! @param[in] channel GPIO channel
//! @param[in] family_code
//! @param[out] device_list Pointer to destination array holding list of devices
//! @param[in] device_list_size Size of array holding list of devices
//! @return Number of found devices

int bc_onewire_search_family(bc_gpio_channel_t channel, uint8_t family_code, uint64_t *device_list, size_t device_list_size);

//! @brief Start of manual search, see also bc_onewire_search_next
//! @param[in] family_code Family code of 1-Wire device or NULL

void bc_onewire_search_start(uint8_t family_code);

//! @brief Manual search of next device
//! @param[in] device_number GPIO channel
//! @param[in] device_number 64b device number
//! @return true if new device was found, false if ther are no more devices on the bus

bool bc_onewire_search_next(bc_gpio_channel_t channel, uint64_t *device_number);

//! @brief Enable call sleep mode for all ds28e17 after transaction
//! @param[in] on

void bc_onewire_auto_ds28e17_sleep_mode(bool on);

//! @brief Calculate 8-bit CRC
//! @param[in] buffer
//! @param[in] length Number of bytes
//! @param[in] The crc starting value
//! @return Calculated CRC

uint8_t bc_onewire_crc8(const void *buffer, size_t length, uint8_t crc);

//! @brief Calculate 16-bit CRC, polynomial 0x8005
//! @param[in] buffer
//! @param[in] length Number of bytes
//! @param[in] The crc starting value
//! @return Calculated CRC

uint16_t bc_onewire_crc16(const void *buffer, size_t length, uint16_t crc);

//! @}

#endif // _BC_ONEWIRE_H
