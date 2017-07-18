#ifndef _BC_DS28E17f
#define _BC_DS28E17_H

#include <bc_1wire.h>
#include <bc_gpio.h>
#include <bc_i2c.h>

//! @addtogroup bc_i2c bc_i2c
//! @brief Driver for I2C bus
//! @{

//! @brief DS28E17 instance

typedef struct bc_ds28e17_t {
	uint64_t _device_number;
	bc_gpio_channel_t _channel;

} bc_ds28e17_t;

//! @brief Initialize DS28E17
//! @param[in] self Instance
//! @param[in] device_number
//! @param[in] channel

void bc_ds28e17_init(bc_ds28e17_t *self, uint64_t device_number, bc_gpio_channel_t channel);

//! @brief Set speed I2C
//! @param[in] self Instance
//! @param[in] speed I2C communication speed

bool bc_ds28e17_set_speed(bc_ds28e17_t *self, bc_i2c_speed_t speed);

//! @brief Write to I2C
//! @param[in] self Instance
//! @param[in] transfer Pointer to I2C transfer parameters instance
//! @return true on success
//! @return false on failure

bool bc_ds28e17_write(bc_ds28e17_t *self, const bc_i2c_transfer_t *transfer);

//! @brief Read from I2C
//! @param[in] self Instance
//! @param[in] transfer Pointer to I2C transfer parameters instance
//! @return true on success
//! @return false on failure

bool bc_ds28e17_read(bc_ds28e17_t *self, const bc_i2c_transfer_t *transfer);

//! @brief Memory write to I2C
//! @param[in] self Instance
//! @param[in] transfer Pointer to I2C memory transfer parameters instance
//! @return true on success
//! @return false on failure

bool bc_ds28e17_memory_write(bc_ds28e17_t *self, const bc_i2c_memory_transfer_t *transfer);

//! @brief Memory read from I2C
//! @param[in] self Instance
//! @param[in] transfer Pointer to I2C memory transfer parameters instance
//! @return true on success
//! @return false on failure

bool bc_ds28e17_memory_read(bc_ds28e17_t *self, const bc_i2c_memory_transfer_t *transfer);

//! @}

#endif /* _BC_DS28E17_H */
