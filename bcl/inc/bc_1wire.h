#ifndef _BC_1WIRE_H
#define _BC_1WIRE_H

#include <bc_gpio.h>

//! @addtogroup bc_onewire bc_onewire
//! @brief Driver for 1-Wire
//! @{

#define DEVICE_NUMBER_SKIP_ROM 0

//! @brief 1-Wire instance

typedef struct
{
    bc_gpio_channel_t _gpio_channel;
    uint8_t _last_discrepancy;
    uint8_t _last_family_discrepancy;
    bool _last_device_flag;
    uint8_t _last_rom_no[8];

} bc_1wire_search_t;

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

void bc_1wire_write(bc_gpio_channel_t channel, void *buffer, size_t length);

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

//! @brief Initialize 1-Wire search
//! @param[in] self Instance
//! @param channel GPIO channel

void bc_1wire_search_init(bc_1wire_search_t *self, bc_gpio_channel_t channel);

//! @brief Reset the search state
//! @param[in] self Instance
//! @param[out] data Output bit which have been read

void bc_1wire_search_reset(bc_1wire_search_t *self);

//! @brief Setup the search to find the device type 'family code' on the next call
//! @param[in] self Instance
//! @param[in] family_code Family code

void bc_1wire_search_target_setup(bc_1wire_search_t *self, uint8_t family_code);

//! @brief Perform the 1-Wire Search Algorithm on the 1-Wire bus using the existing search state.
//! @param[in] self Instance
//! @param[out] device_number Device number
//! @return true  Device found
//! @return false Device not found, end of search

bool bc_1wire_search(bc_1wire_search_t *self, uint64_t *device_number);

//! @brief Calculate crc8
//! @param[in] buffer
//! @param[in] length Number of bytes
//! @return crc

uint8_t bc_1wire_crc8(void *buffer, size_t length);

//! @brief Calculate crc16
//! @param[in] buffer
//! @param[in] length Number of bytes
//! @param[in] The crc starting value
//! @return crc

uint16_t bc_1wire_crc16(const void *buffer, uint16_t length, uint16_t crc);

//! @}

#endif /* _BC_1WIRE_H */
