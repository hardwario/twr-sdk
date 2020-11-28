#ifndef _HIO_DS28E17_H
#define _HIO_DS28E17_H

#include <hio_onewire.h>
#include <hio_i2c.h>

//! @addtogroup hio_ds28e17 hio_ds28e17
//! @brief Driver for DS28E17 (1-wire-to-I2C Master Bridge)
//! @{

//! @brief DS28E17 instance

typedef struct
{
    uint64_t _device_number;
    hio_onewire_t *_onewire;

} hio_ds28e17_t;

//! @brief Initialize DS28E17
//! @param[in] self Instance
//! @param[in] Pointer on instance 1-Wire
//! @param[in] device_number Device number

void hio_ds28e17_init(hio_ds28e17_t *self, hio_onewire_t *onewire, uint64_t device_number);

//! @brief Deinitialize DS28E17
//! @param[in] self Instance

void hio_ds28e17_deinit(hio_ds28e17_t *self);

//! @brief Get Device number
//! @param[in] self Instance

uint64_t hio_ds28e17_get_device_number(hio_ds28e17_t *self);

//! @brief Enable Sleep Mode
//! @param[in] self Instance

bool hio_ds28e17_enable_sleep_mode(hio_ds28e17_t *self);

//! @brief Set I2C speed
//! @param[in] self Instance
//! @param[in] speed I2C communication speed

bool hio_ds28e17_set_speed(hio_ds28e17_t *self, hio_i2c_speed_t speed);

//! @brief Write to I2C
//! @param[in] self Instance
//! @param[in] transfer Pointer to I2C transfer parameters instance
//! @return true On success
//! @return false On failure

bool hio_ds28e17_write(hio_ds28e17_t *self, const hio_i2c_transfer_t *transfer);

//! @brief Read from I2C
//! @param[in] self Instance
//! @param[in] transfer Pointer to I2C transfer parameters instance
//! @return true On success
//! @return false On failure

bool hio_ds28e17_read(hio_ds28e17_t *self, const hio_i2c_transfer_t *transfer);

//! @brief Memory write to I2C
//! @param[in] self Instance
//! @param[in] transfer Pointer to I2C memory transfer parameters instance
//! @return true On success
//! @return false On failure

bool hio_ds28e17_memory_write(hio_ds28e17_t *self, const hio_i2c_memory_transfer_t *transfer);

//! @brief Memory read from I2C
//! @param[in] self Instance
//! @param[in] transfer Pointer to I2C memory transfer parameters instance
//! @return true On success
//! @return false On failure

bool hio_ds28e17_memory_read(hio_ds28e17_t *self, const hio_i2c_memory_transfer_t *transfer);

//! @}

#endif // _HIO_DS28E17_H
