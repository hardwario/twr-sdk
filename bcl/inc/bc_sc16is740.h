#ifndef _BC_SC16IS740_H
#define _BC_SC16IS740_H

#include <bc_i2c.h>
#include <bc_tick.h>

//! @addtogroup bc_tca9534a bc_tca9534a
//! @brief Driver for SC16IS740 single UART with I2C-bus interface, 64 bytes of transmit and receive FIFOs
//! @{

//! @brief Fifo type

typedef enum
{
    BC_SC16IS740_FIFO_RX = 0x02,
    BC_SC16IS740_FIFO_TX = 0x04

} bc_sc16is740_fifo_t;

//! @brief TCA9534A instance

typedef struct
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;

} bc_sc16is740_t;

//! @brief Initialize SC16IS740
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address
//! @return true on success
//! @return false on failure

bool bc_sc16is740_init(bc_sc16is740_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Reset FIFO
//! @param[in] self Instance
//! @param[in] fifo
//! @return true on success
//! @return false on failure

bool bc_sc16is740_reset_fifo(bc_sc16is740_t *self, bc_sc16is740_fifo_t fifo);

//! @brief Get TX FIXO space available
//! @param[in] self Instance
//! @param[out] spaces_available
//! @return true on success
//! @return false on failure

bool bc_sc16is740_get_spaces_available(bc_sc16is740_t *self, uint8_t *spaces_available);

//! @brief Write
//! @param[in] self Instance
//! @param[in] buffer Pointer to source buffer
//! @param[in] length Number of bytes to be written
//! @return Number of bytes written

uint8_t bc_sc16is740_write(bc_sc16is740_t *self, uint8_t *buffer, uint8_t length);

//! @brief Get RX FIXO available data
//! @param[in] self Instance
//! @param[out] available
//! @return true on success
//! @return false on failure

bool bc_sc16is740_available(bc_sc16is740_t *self, uint8_t *available);

//! @brief Read
//! @param[in] self Instance
//! @param[out] buffer Pointer to destination buffer
//! @param[in] length Number of bytes to be read
//! @param[in] timeout Write operation timeout in ticks
//! @return Number of bytes read

uint8_t bc_sc16is740_read(bc_sc16is740_t *self, uint8_t *buffer, uint8_t length, bc_tick_t timeout);

//! @}

#endif /* _BC_SC16IS740_H */
