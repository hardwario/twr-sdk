#ifndef _HIO_I2C_H
#define _HIO_I2C_H

#include <hio_common.h>

//! @addtogroup hio_i2c hio_i2c
//! @brief Driver for I2C bus
//! @{

//! @brief This flag extends I2C memory transfer address from 8-bit to 16-bit
#define HIO_I2C_MEMORY_ADDRESS_16_BIT 0x80000000

//! @brief I2C channels

typedef enum
{
    //! @brief I2C channel I2C0
    HIO_I2C_I2C0 = 0,

    //! @brief I2C channel I2C1
    HIO_I2C_I2C1 = 1,

    //! @brief I2C channel 1wire
    HIO_I2C_I2C_1W = 2

} hio_i2c_channel_t;

//! @brief I2C communication speed

typedef enum
{
    //! @brief I2C communication speed is 100 kHz
    HIO_I2C_SPEED_100_KHZ = 0,

    //! @brief I2C communication speed is 400 kHz
    HIO_I2C_SPEED_400_KHZ = 1

} hio_i2c_speed_t;

//! @brief I2C transfer parameters

typedef struct
{
    //! @brief 7-bit I2C device address
    uint8_t device_address;

    //! @brief Pointer to buffer which is being written or read
    void *buffer;

    //! @brief Length of buffer which is being written or read
    size_t length;

} hio_i2c_transfer_t;

//! @brief I2C memory transfer parameters

typedef struct
{
    //! @brief 7-bit I2C device address
    uint8_t device_address;

    //! @brief 8-bit I2C memory address (it can be extended to 16-bit format if OR-ed with HIO_I2C_MEMORY_ADDRESS_16_BIT)
    uint32_t memory_address;

    //! @brief Pointer to buffer which is being written or read
    void *buffer;

    //! @brief Length of buffer which is being written or read
    size_t length;

} hio_i2c_memory_transfer_t;

//! @brief Initialize I2C channel
//! @param[in] channel I2C channel
//! @param[in] speed I2C communication speed

void hio_i2c_init(hio_i2c_channel_t channel, hio_i2c_speed_t speed);

//! @brief Deitialize I2C channel
//! @param[in] channel I2C channel

void hio_i2c_deinit(hio_i2c_channel_t channel);

//! @brief Get speed I2C channel
//! @param[in] channel I2C channel
//! @return I2C communication speed

hio_i2c_speed_t hio_i2c_get_speed(hio_i2c_channel_t channel);

//! @brief Set I2C channel speed
//! @param[in] channel I2C channel
//! @param[in] speed I2C communication speed

void hio_i2c_set_speed(hio_i2c_channel_t channel, hio_i2c_speed_t speed);

//! @brief Write to I2C channel
//! @param[in] channel I2C channel
//! @param[in] transfer Pointer to I2C transfer parameters instance
//! @return true On success
//! @return false On failure

bool hio_i2c_write(hio_i2c_channel_t channel, const hio_i2c_transfer_t *transfer);

//! @brief Read from I2C channel
//! @param[in] channel I2C channel
//! @param[in] transfer Pointer to I2C transfer parameters instance
//! @return true On success
//! @return false On failure

bool hio_i2c_read(hio_i2c_channel_t channel, const hio_i2c_transfer_t *transfer);

//! @brief Memory write to I2C channel
//! @param[in] channel I2C channel
//! @param[in] transfer Pointer to I2C memory transfer parameters instance
//! @return true On success
//! @return false On failure

bool hio_i2c_memory_write(hio_i2c_channel_t channel, const hio_i2c_memory_transfer_t *transfer);

//! @brief Memory read from I2C channel
//! @param[in] channel I2C channel
//! @param[in] transfer Pointer to I2C memory transfer parameters instance
//! @return true On success
//! @return false On failure

bool hio_i2c_memory_read(hio_i2c_channel_t channel, const hio_i2c_memory_transfer_t *transfer);

//! @brief Memory write 1 byte to I2C channel
//! @param[in] channel I2C channel
//! @param[in] device_address 7-bit I2C device address
//! @param[in] memory_address 8-bit I2C memory address (it can be extended to 16-bit format if OR-ed with HIO_I2C_MEMORY_ADDRESS_16_BIT)
//! @param[in] data Input data to be written

bool hio_i2c_memory_write_8b(hio_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint8_t data);

//! @brief Memory write 2 bytes to I2C channel
//! @param[in] channel I2C channel
//! @param[in] device_address 7-bit I2C device address
//! @param[in] memory_address 8-bit I2C memory address (it can be extended to 16-bit format if OR-ed with HIO_I2C_MEMORY_ADDRESS_16_BIT)
//! @param[in] data Input data to be written (MSB first)

bool hio_i2c_memory_write_16b(hio_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint16_t data);

//! @brief Memory read 1 byte from I2C channel
//! @param[in] channel I2C channel
//! @param[in] device_address 7-bit I2C device address
//! @param[in] memory_address 8-bit I2C memory address (it can be extended to 16-bit format if OR-ed with HIO_I2C_MEMORY_ADDRESS_16_BIT)
//! @param[out] data Output data which have been read

bool hio_i2c_memory_read_8b(hio_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint8_t *data);

//! @brief Memory read 2 bytes from I2C channel
//! @param[in] channel I2C channel
//! @param[in] device_address 7-bit I2C device address
//! @param[in] memory_address 8-bit I2C memory address (it can be extended to 16-bit format if OR-ed with HIO_I2C_MEMORY_ADDRESS_16_BIT)
//! @param[out] data Output data which have been read (MSB first)

bool hio_i2c_memory_read_16b(hio_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint16_t *data);

//! @}

#endif // _HIO_I2C_H
