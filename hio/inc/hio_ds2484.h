#ifndef _HIO_DS2484_H
#define _HIO_DS2484_H

#include <hio_i2c.h>

//! @addtogroup hio_ds2484 hio_ds2484
//! @brief Driver for DS2484 Single-Channel 1-Wire Master
//! @{

//! @brief Callback events

#define	HIO_DS2484_STATUS_1WB   (1<<0) // 1-Wire Busy
#define	HIO_DS2484_STATUS_PPD   (1<<1) // Presence Pulse Detect
#define	HIO_DS2484_STATUS_SD    (1<<2) // Short Detected
#define	HIO_DS2484_STATUS_LL    (1<<3) // Logic Level
#define	HIO_DS2484_STATUS_RST   (1<<4) // Device Reset
#define	HIO_DS2484_STATUS_SBR   (1<<5) // Single Bit Result
#define	HIO_DS2484_STATUS_TSB   (1<<6) // Triplet Second Bit
#define	HIO_DS2484_STATUS_DIR   (1<<7) // Branch Direction Taken

//! @brief TMP112 instance

typedef struct hio_ds2484_t hio_ds2484_t;

//! @cond

struct hio_ds2484_t
{
    bool _ready;
    bool _spu_on;
    uint8_t _status;
    uint8_t _srp;
    hio_i2c_channel_t _i2c_channel;
};

//! @endcond

//! @brief Initialize DS2484
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel

bool hio_ds2484_init(hio_ds2484_t *self, hio_i2c_channel_t i2c_channel);

//! @brief Enable DS2484
//! @param[in] self Instance

void hio_ds2484_enable(hio_ds2484_t *self);

//! @brief Disable DS2484
//! @param[in] self Instance

void hio_ds2484_disable(hio_ds2484_t *self);

//! @brief Reset the 1-Wire bus and return the presence of any device
//! @param[in] self Instance

bool hio_ds2484_reset(hio_ds2484_t *self);

//! @brief Wait until not busy
//! @param[in] self Instance

bool hio_ds2484_busy_wait(hio_ds2484_t *self);

bool hio_ds2484_write_byte(hio_ds2484_t *self, const uint8_t byte);

bool hio_ds2484_read_byte(hio_ds2484_t *self, uint8_t *byte);

bool hio_ds2484_read_bit(hio_ds2484_t *self, uint8_t *bit);

bool hio_ds2484_triplet(hio_ds2484_t *self, const uint8_t direction);

bool hio_ds2484_is_ready(hio_ds2484_t *self);

uint8_t hio_ds2484_status_get(hio_ds2484_t *self);

bool hio_ds2484_is_present(hio_ds2484_t *self);

//! @}

#endif // _HIO_HIO_DS2484_H
