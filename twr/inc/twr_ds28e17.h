#ifndef _TWR_DS28E17_H
#define _TWR_DS28E17_H

#include <twr_onewire.h>
#include <twr_i2c.h>

//! @addtogroup twr_ds28e17 twr_ds28e17
//! @brief Driver for DS28E17 (1-wire-to-I2C Master Bridge)
//! @{

//! @brief DS28E17 instance

typedef struct
{
    uint64_t _device_number;
    twr_onewire_t *_onewire;

} twr_ds28e17_t;

//! @brief Initialize DS28E17
//! @param[in] self Instance
//! @param[in] Pointer on instance 1-Wire
//! @param[in] device_number Device number

void twr_ds28e17_init(twr_ds28e17_t *self, twr_onewire_t *onewire, uint64_t device_number);

//! @brief Deinitialize DS28E17
//! @param[in] self Instance

void twr_ds28e17_deinit(twr_ds28e17_t *self);

//! @brief Get Device number
//! @param[in] self Instance

uint64_t twr_ds28e17_get_device_number(twr_ds28e17_t *self);

//! @brief Enable Sleep Mode
//! @param[in] self Instance

bool twr_ds28e17_enable_sleep_mode(twr_ds28e17_t *self);

//! @brief Set I2C speed
//! @param[in] self Instance
//! @param[in] speed I2C communication speed

bool twr_ds28e17_set_speed(twr_ds28e17_t *self, twr_i2c_speed_t speed);

//! @brief Write to I2C
//! @param[in] self Instance
//! @param[in] transfer Pointer to I2C transfer parameters instance
//! @return true On success
//! @return false On failure

bool twr_ds28e17_write(twr_ds28e17_t *self, const twr_i2c_transfer_t *transfer);

//! @brief Read from I2C
//! @param[in] self Instance
//! @param[in] transfer Pointer to I2C transfer parameters instance
//! @return true On success
//! @return false On failure

bool twr_ds28e17_read(twr_ds28e17_t *self, const twr_i2c_transfer_t *transfer);

//! @brief Memory write to I2C
//! @param[in] self Instance
//! @param[in] transfer Pointer to I2C memory transfer parameters instance
//! @return true On success
//! @return false On failure

bool twr_ds28e17_memory_write(twr_ds28e17_t *self, const twr_i2c_memory_transfer_t *transfer);

//! @brief Memory read from I2C
//! @param[in] self Instance
//! @param[in] transfer Pointer to I2C memory transfer parameters instance
//! @return true On success
//! @return false On failure

bool twr_ds28e17_memory_read(twr_ds28e17_t *self, const twr_i2c_memory_transfer_t *transfer);

//! @}

#endif // _TWR_DS28E17_H
