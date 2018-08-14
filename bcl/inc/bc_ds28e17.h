#ifndef _BC_DS28E17_H
#define _BC_DS28E17_H

#include <bc_onewire.h>
#include <bc_gpio.h>
#include <bc_i2c.h>

//! @addtogroup bc_ds28e17 bc_ds28e17
//! @brief Driver for DS28E17 (1-wire-to-I2C Master Bridge)
//! @{

//! @brief DS28E17 instance

typedef struct
{
    uint64_t _device_number;
    bc_gpio_channel_t _channel;

} bc_ds28e17_t;

//! @brief Initialize DS28E17
//! @param[in] self Instance
//! @param[in] channel GPIO channel
//! @param[in] device_number Device number

void bc_ds28e17_init(bc_ds28e17_t *self, bc_gpio_channel_t channel, uint64_t device_number);

//! @brief Deinitialize DS28E17
//! @param[in] self Instance

void bc_ds28e17_deinit(bc_ds28e17_t *self);

//! @brief Get Device number
//! @param[in] self Instance

uint64_t bc_ds28e17_get_device_number(bc_ds28e17_t *self);

//! @brief Enable Sleep Mode
//! @param[in] self Instance

bool bc_ds28e17_enable_sleep_mode(bc_ds28e17_t *self);

//! @brief Set I2C speed
//! @param[in] self Instance
//! @param[in] speed I2C communication speed

bool bc_ds28e17_set_speed(bc_ds28e17_t *self, bc_i2c_speed_t speed);

//! @brief Write to I2C
//! @param[in] self Instance
//! @param[in] transfer Pointer to I2C transfer parameters instance
//! @return true On success
//! @return false On failure

bool bc_ds28e17_write(bc_ds28e17_t *self, const bc_i2c_transfer_t *transfer);

//! @brief Read from I2C
//! @param[in] self Instance
//! @param[in] transfer Pointer to I2C transfer parameters instance
//! @return true On success
//! @return false On failure

bool bc_ds28e17_read(bc_ds28e17_t *self, const bc_i2c_transfer_t *transfer);

//! @brief Memory write to I2C
//! @param[in] self Instance
//! @param[in] transfer Pointer to I2C memory transfer parameters instance
//! @return true On success
//! @return false On failure

bool bc_ds28e17_memory_write(bc_ds28e17_t *self, const bc_i2c_memory_transfer_t *transfer);

//! @brief Memory read from I2C
//! @param[in] self Instance
//! @param[in] transfer Pointer to I2C memory transfer parameters instance
//! @return true On success
//! @return false On failure

bool bc_ds28e17_memory_read(bc_ds28e17_t *self, const bc_i2c_memory_transfer_t *transfer);

//! @}

#endif // _BC_DS28E17_H
