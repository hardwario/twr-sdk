#ifndef _TWR_DS2484_H
#define _TWR_DS2484_H

#include <twr_i2c.h>

//! @addtogroup twr_ds2484 twr_ds2484
//! @brief Driver for DS2484 Single-Channel 1-Wire Master
//! @{

//! @brief Callback events

#define	TWR_DS2484_STATUS_1WB   (1<<0) // 1-Wire Busy
#define	TWR_DS2484_STATUS_PPD   (1<<1) // Presence Pulse Detect
#define	TWR_DS2484_STATUS_SD    (1<<2) // Short Detected
#define	TWR_DS2484_STATUS_LL    (1<<3) // Logic Level
#define	TWR_DS2484_STATUS_RST   (1<<4) // Device Reset
#define	TWR_DS2484_STATUS_SBR   (1<<5) // Single Bit Result
#define	TWR_DS2484_STATUS_TSB   (1<<6) // Triplet Second Bit
#define	TWR_DS2484_STATUS_DIR   (1<<7) // Branch Direction Taken

//! @brief TMP112 instance

typedef struct twr_ds2484_t twr_ds2484_t;

//! @cond

struct twr_ds2484_t
{
    bool _ready;
    bool _spu_on;
    uint8_t _status;
    uint8_t _srp;
    twr_i2c_channel_t _i2c_channel;
    bool (*_set_slpz)(void *, bool);
    void *_set_slpz_ctx;
};

//! @endcond

//! @brief Initialize DS2484
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel

bool twr_ds2484_init(twr_ds2484_t *self, twr_i2c_channel_t i2c_channel);

void twr_ds2484_set_slpz_handler(twr_ds2484_t *self, bool (*handler)(void *, bool), void *handler_ctx);

//! @brief Enable DS2484
//! @param[in] self Instance

void twr_ds2484_enable(twr_ds2484_t *self);

//! @brief Disable DS2484
//! @param[in] self Instance

void twr_ds2484_disable(twr_ds2484_t *self);

//! @brief Reset the 1-Wire bus and return the presence of any device
//! @param[in] self Instance

bool twr_ds2484_reset(twr_ds2484_t *self);

//! @brief Wait until not busy
//! @param[in] self Instance

bool twr_ds2484_busy_wait(twr_ds2484_t *self);

bool twr_ds2484_write_byte(twr_ds2484_t *self, const uint8_t byte);

bool twr_ds2484_read_byte(twr_ds2484_t *self, uint8_t *byte);

bool twr_ds2484_read_bit(twr_ds2484_t *self, uint8_t *bit);

bool twr_ds2484_triplet(twr_ds2484_t *self, const uint8_t direction);

bool twr_ds2484_is_ready(twr_ds2484_t *self);

uint8_t twr_ds2484_status_get(twr_ds2484_t *self);

bool twr_ds2484_is_present(twr_ds2484_t *self);

//! @}

#endif // _TWR_TWR_DS2484_H
