#ifndef _HIO_SC16IS740_H
#define _HIO_SC16IS740_H

#include <hio_i2c.h>
#include <hio_tick.h>

//! @addtogroup hio_sc16is740 hio_sc16is740
//! @brief Driver for SC16IS740 single UART with I2C-bus interface, 64 bytes of transmit and receive FIFOs
//! @{

//! @brief Fifo type

typedef enum
{
    HIO_SC16IS740_FIFO_RX = 0x02,
    HIO_SC16IS740_FIFO_TX = 0x04

} hio_sc16is740_fifo_t;

//! @brief Baudrates

typedef enum
{
    HIO_SC16IS740_BAUDRATE_9600 = 88,
    HIO_SC16IS740_BAUDRATE_19200 = 44,
    HIO_SC16IS740_BAUDRATE_38400 = 22,
    HIO_SC16IS740_BAUDRATE_57600 = 15,
    HIO_SC16IS740_BAUDRATE_115200 = 7

} hio_sc16is740_baudrate_t;

//! @brief SC16IS740 instance

//! @cond

typedef struct
{
    hio_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;

} hio_sc16is740_t;

//! @endcond

//! @brief Initialize SC16IS740
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address
//! @return true On success
//! @return false On failure

bool hio_sc16is740_init(hio_sc16is740_t *self, hio_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Reset FIFO
//! @param[in] self Instance
//! @param[in] fifo
//! @return true On success
//! @return false On failure

bool hio_sc16is740_reset_fifo(hio_sc16is740_t *self, hio_sc16is740_fifo_t fifo);

//! @brief Get TX FIXO space available
//! @param[in] self Instance
//! @param[out] spaces_available
//! @return true On success
//! @return false On failure

bool hio_sc16is740_get_spaces_available(hio_sc16is740_t *self, size_t *spaces_available);

//! @brief Write
//! @param[in] self Instance
//! @param[in] buffer Pointer to source buffer
//! @param[in] length Number of bytes to be written
//! @return Number of bytes written

size_t hio_sc16is740_write(hio_sc16is740_t *self, uint8_t *buffer, size_t length);

//! @brief Get RX FIXO available data
//! @param[in] self Instance
//! @param[out] available
//! @return true On success
//! @return false On failure

bool hio_sc16is740_available(hio_sc16is740_t *self, size_t *available);

//! @brief Read
//! @param[in] self Instance
//! @param[out] buffer Pointer to destination buffer
//! @param[in] length Number of bytes to be read
//! @param[in] timeout Write operation timeout in ticks
//! @return Number of bytes read

size_t hio_sc16is740_read(hio_sc16is740_t *self, uint8_t *buffer, size_t length, hio_tick_t timeout);

//! @brief Set baudrate
//! @param[in] self Instance
//! @param[in] baudrate
//! @return true On success
//! @return false On failure

bool hio_sc16is740_set_baudrate(hio_sc16is740_t *self, hio_sc16is740_baudrate_t baudrate);

//! @}

#endif // _HIO_SC16IS740_H
