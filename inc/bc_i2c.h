#ifndef _BC_I2C_H
#define _BC_I2C_H

#include <bc_common.h>

//! @addtogroup bc_i2c bc_i2c
//! @brief Driver for I2C bus
//! @{

//! @brief This flag extends I2C transfer memory address from 8-bit to 16-bit
#define BC_I2C_MEMORY_ADDRESS_16_BIT 0x80000000UL

//! @brief I2C channels

typedef enum
{
    BC_I2C_I2C0 = 0L, //!< I2C channel I2C0
    BC_I2C_I2C1 = 1L  //!< I2C channel I2C1

} bc_i2c_channel_t;

//! @brief I2C communication speed

typedef enum
{
    BC_I2C_SPEED_100_KHZ = 0L, //!< I2C communication speed is 100 kHz
    BC_I2C_SPEED_400_KHZ = 1L  //!< I2C communication speed is 400 kHz

} bc_i2c_speed_t;

//! @brief I2C transfer parameters

typedef struct
{
    //! @brief 7-bit I2C device address
    uint8_t device_address;

    //! @brief 8-bit I2C memory address (it can be extended to 16-bit format if OR-ed with BC_I2C_MEMORY_ADDRESS_16_BIT)
    uint32_t memory_address;

    //! @brief Pointer to buffer which is being written or read
    void *buffer;

    //! @brief Length of buffer which is being written or read
    size_t length;

} bc_i2c_tranfer_t;

//! @brief Initialize I2C channel
//! @param[in] channel I2C channel
//! @param[in] speed I2C communication speed

void bc_i2c_init(bc_i2c_channel_t channel, bc_i2c_speed_t speed);

//! @brief Write to I2C channel
//! @param[in] channel I2C channel
//! @param[in] transfer Pointer to I2C transfer parameters instance
//! @return true on success
//! @return false on failure

bool bc_i2c_write(bc_i2c_channel_t channel, const bc_i2c_tranfer_t *transfer);

//! @brief Read from I2C channel
//! @param[in] channel I2C channel
//! @param[in] transfer Pointer to I2C transfer parameters instance
//! @return true on success
//! @return false on failure

bool bc_i2c_read(bc_i2c_channel_t channel, const bc_i2c_tranfer_t *transfer);

//! @brief Write 1 byte to I2C channel
//! @param[in] channel I2C channel
//! @param[in] device_address 7-bit I2C device address
//! @param[in] memory_address 8-bit I2C memory address (it can be extended to 16-bit format if OR-ed with BC_I2C_MEMORY_ADDRESS_16_BIT)
//! @param[in] data Input data to be written

bool bc_i2c_write_8b(bc_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint8_t data);

//! @brief Write 2 bytes to I2C channel
//! @param[in] channel I2C channel
//! @param[in] device_address 7-bit I2C device address
//! @param[in] memory_address 8-bit I2C memory address (it can be extended to 16-bit format if OR-ed with BC_I2C_MEMORY_ADDRESS_16_BIT)
//! @param[in] data Input data to be written (MSB first)

bool bc_i2c_write_16b(bc_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint16_t data);

//! @brief Read 1 byte from I2C channel
//! @param[in] channel I2C channel
//! @param[in] device_address 7-bit I2C device address
//! @param[in] memory_address 8-bit I2C memory address (it can be extended to 16-bit format if OR-ed with BC_I2C_MEMORY_ADDRESS_16_BIT)
//! @param[out] data Output data which have been read

bool bc_i2c_read_8b(bc_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint8_t *data);

//! @brief Read 2 bytes from I2C channel
//! @param[in] channel I2C channel
//! @param[in] device_address 7-bit I2C device address
//! @param[in] memory_address 8-bit I2C memory address (it can be extended to 16-bit format if OR-ed with BC_I2C_MEMORY_ADDRESS_16_BIT)
//! @param[out] data Output data which have been read (MSB first)

bool bc_i2c_read_16b(bc_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint16_t *data);

//! @}

#endif // _BC_I2C_H
