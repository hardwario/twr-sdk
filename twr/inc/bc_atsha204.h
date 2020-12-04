#ifndef _BC_ATSHA204_H
#define _BC_ATSHA204_H

#include <bc_common.h>
#include <bc_i2c.h>
#include <bc_scheduler.h>

#define BC_ATSHA204_RX_BUFFER_SIZE 32

//! @addtogroup bc_atsha204 bc_atsha204
//! @brief Driver for Atmel ATSHA204 CryptoAuthentication
//! @{

typedef enum
{
    //! @brief Error event
    BC_ATSHA204_EVENT_ERROR = 0,

    //! @brief Event revision number is available
    BC_ATSHA204_EVENT_REVISION_NUMBER = 1,

    //! @brief Event serial number is available
    BC_ATSHA204_EVENT_SERIAL_NUMBER = 2

} bc_atsha204_event_t;

//! @brief ATSHA204 instance

typedef struct bc_atsha204_t bc_atsha204_t;

//! @cond

typedef enum
{
    BC_ATSHA204_STATE_READY = 0,
    BC_ATSHA204_STATE_READ_SERIAL_NUMBER = 1,
    BC_ATSHA204_STATE_READ_SERIAL_NUMBER2 = 2,
    BC_ATSHA204_STATE_SERIAL_NUMBER = 3

} bc_atsha204_state_t;

struct bc_atsha204_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    void (*_event_handler)(bc_atsha204_t *, bc_atsha204_event_t, void *);
    void *_event_param;
    bc_scheduler_task_id_t _task_id;
    bc_atsha204_state_t _state;
    bool _ready;
    uint8_t _rx_buffer[BC_ATSHA204_RX_BUFFER_SIZE];

};

//! @endcond

//! @brief Initialize ATSHA204 driver
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void bc_atsha204_init(bc_atsha204_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_atsha204_set_event_handler(bc_atsha204_t *self, void (*event_handler)(bc_atsha204_t *, bc_atsha204_event_t, void *), void *event_param);

//! @brief Check if is ready for commands
//! @param[in] self Instance
//! @return true If ready
//! @return false If not ready

bool bc_atsha204_is_ready(bc_atsha204_t *self);

//! @brief Reqeust for serial number
//! @param[in] self Instance
//! @return true On success
//! @return False When other command is in progress

bool bc_atsha204_read_serial_number(bc_atsha204_t *self);

//! @brief Get serial number
//! @param[in] self Instance
//! @param[out] destination Pointer to destination object where device unique ID will be stored
//! @param[in] size Size of destination object (in bytes)
//! @return true When value is valid
//! @return false When value is invalid

bool bc_atsha204_get_serial_number(bc_atsha204_t *self, void *destination, size_t size);

//! @}

#endif // _BC_ATSHA204_H

