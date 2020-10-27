#ifndef _BC_DS2484_H
#define _BC_DS2484_H

#include <bc_i2c.h>

//! @addtogroup bc_ds2484 bc_ds2484
//! @brief Driver for DS2484 Single-Channel 1-Wire Master
//! @{

//! @brief Callback events

#define	BC_DS2484_STATUS_1WB   (1<<0) // 1-Wire Busy
#define	BC_DS2484_STATUS_PPD   (1<<1) // Presence Pulse Detect
#define	BC_DS2484_STATUS_SD    (1<<2) // Short Detected
#define	BC_DS2484_STATUS_LL    (1<<3) // Logic Level
#define	BC_DS2484_STATUS_RST   (1<<4) // Device Reset
#define	BC_DS2484_STATUS_SBR   (1<<5) // Single Bit Result
#define	BC_DS2484_STATUS_TSB   (1<<6) // Triplet Second Bit
#define	BC_DS2484_STATUS_DIR   (1<<7) // Branch Direction Taken

//! @brief TMP112 instance

typedef struct bc_ds2484_t bc_ds2484_t;

//! @cond

struct bc_ds2484_t
{
    bool _ready;
    bool _spu_on;
    uint8_t _status;
    uint8_t _srp;
    bc_i2c_channel_t _i2c_channel;
};

//! @endcond

//! @brief Initialize DS2484
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel

bool bc_ds2484_init(bc_ds2484_t *self, bc_i2c_channel_t i2c_channel);

//! @brief Enable DS2484
//! @param[in] self Instance

void bc_ds2484_enable(bc_ds2484_t *self);

//! @brief Disable DS2484
//! @param[in] self Instance

void bc_ds2484_disable(bc_ds2484_t *self);

//! @brief Reset the 1-Wire bus and return the presence of any device
//! @param[in] self Instance

bool bc_ds2484_reset(bc_ds2484_t *self);

//! @brief Wait until not busy
//! @param[in] self Instance

bool bc_ds2484_busy_wait(bc_ds2484_t *self);

bool bc_ds2484_write_byte(bc_ds2484_t *self, const uint8_t byte);

bool bc_ds2484_read_byte(bc_ds2484_t *self, uint8_t *byte);

bool bc_ds2484_read_bit(bc_ds2484_t *self, uint8_t *bit);

bool bc_ds2484_triplet(bc_ds2484_t *self, const uint8_t direction);

bool bc_ds2484_is_ready(bc_ds2484_t *self);

uint8_t bc_ds2484_status_get(bc_ds2484_t *self);

bool bc_ds2484_is_present(bc_ds2484_t *self);

//! @}

#endif // _BC_BC_DS2484_H
