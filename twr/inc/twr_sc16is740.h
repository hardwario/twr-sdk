#ifndef _TWR_SC16IS740_H
#define _TWR_SC16IS740_H

#include <twr_i2c.h>
#include <twr_tick.h>

//! @addtogroup twr_sc16is740 twr_sc16is740
//! @brief Driver for SC16IS740 single UART with I2C-bus interface, 64 bytes of transmit and receive FIFOs
//! @{

//! @brief Fifo type

typedef enum
{
    TWR_SC16IS740_FIFO_RX = 0x02,
    TWR_SC16IS740_FIFO_TX = 0x04

} twr_sc16is740_fifo_t;

//! @brief Baudrates

typedef enum
{
    TWR_SC16IS740_BAUDRATE_9600 = 88,
    TWR_SC16IS740_BAUDRATE_19200 = 44,
    TWR_SC16IS740_BAUDRATE_38400 = 22,
    TWR_SC16IS740_BAUDRATE_57600 = 15,
    TWR_SC16IS740_BAUDRATE_115200 = 7

} twr_sc16is740_baudrate_t;

//! @brief SC16IS740 instance

//! @cond

typedef struct
{
    twr_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;

} twr_sc16is740_t;

//! @endcond

//! @brief Initialize SC16IS740
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address
//! @return true On success
//! @return false On failure

bool twr_sc16is740_init(twr_sc16is740_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Reset FIFO
//! @param[in] self Instance
//! @param[in] fifo
//! @return true On success
//! @return false On failure

bool twr_sc16is740_reset_fifo(twr_sc16is740_t *self, twr_sc16is740_fifo_t fifo);

//! @brief Get TX FIXO space available
//! @param[in] self Instance
//! @param[out] spaces_available
//! @return true On success
//! @return false On failure

bool twr_sc16is740_get_spaces_available(twr_sc16is740_t *self, size_t *spaces_available);

//! @brief Write
//! @param[in] self Instance
//! @param[in] buffer Pointer to source buffer
//! @param[in] length Number of bytes to be written
//! @return Number of bytes written

size_t twr_sc16is740_write(twr_sc16is740_t *self, uint8_t *buffer, size_t length);

//! @brief Get RX FIXO available data
//! @param[in] self Instance
//! @param[out] available
//! @return true On success
//! @return false On failure

bool twr_sc16is740_available(twr_sc16is740_t *self, size_t *available);

//! @brief Read
//! @param[in] self Instance
//! @param[out] buffer Pointer to destination buffer
//! @param[in] length Number of bytes to be read
//! @param[in] timeout Write operation timeout in ticks
//! @return Number of bytes read

size_t twr_sc16is740_read(twr_sc16is740_t *self, uint8_t *buffer, size_t length, twr_tick_t timeout);

//! @brief Set baudrate
//! @param[in] self Instance
//! @param[in] baudrate
//! @return true On success
//! @return false On failure

bool twr_sc16is740_set_baudrate(twr_sc16is740_t *self, twr_sc16is740_baudrate_t baudrate);

//! @}

#endif // _TWR_SC16IS740_H
