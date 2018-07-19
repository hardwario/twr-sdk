#ifndef _BC_ATSHA204_H
#define _BC_ATSHA204_H

#include <bc_common.h>
#include <bc_i2c.h>
#include <bc_scheduler.h>

//! @addtogroup bc_atsha204 bc_atsha204
//! @brief Driver for Atmel ATSHA204 CryptoAuthentication
//! @{

#define BC_ATSHA204_ZONE_CONFIG_LENGTH 88
#define BC_ATSHA204_ZONE_OTP_LENGTH    64
#define BC_ATSHA204_ZONE_DATA_LENGTH   512

typedef enum
{
    //! @brief Error event
    BC_ATSHA204_EVENT_ERROR = 0,

    //! @brief Event revision number is available
    BC_ATSHA204_EVENT_REVISION_NUMBER = 1,

    //! @brief Event serial number is available
    BC_ATSHA204_EVENT_SERIAL_NUMBER = 2,

    //! @brief Event random value is available
    BC_ATSHA204_EVENT_RANDOM = 3,

    //! @brief Event read done
    BC_ATSHA204_EVENT_READ_DONE = 4,

    //! @brief Event lock done
    BC_ATSHA204_EVENT_LOCK_DONE = 5,

    //! @brief Event lock error
    BC_ATSHA204_EVENT_LOCK_ERROR = 6,

} bc_atsha204_event_t;

typedef enum
{
    //! @brief Zone Config
    BC_ATSHA204_ZONE_CONFIG = 0,

    //! @brief Zone OTP
    BC_ATSHA204_ZONE_OTP = 1,

    //! @brief Zone Data
    BC_ATSHA204_ZONE_DATA = 2,

} bc_atsha204_zone_t;

typedef enum
{
    //! @brief Lock Zone Config
    BC_ATSHA204_LOCK_ZONE_CONFIG = 0,

    //! @brief Lock zone OTP and Data
    BC_ATSHA204_LOCK_ZONE_OTP_DATA = 1,

} bc_atsha204_lock_zone_t;

//! @brief ATSHA204 instance

typedef struct bc_atsha204_t bc_atsha204_t;

//! @cond

typedef enum
{
    BC_ATSHA204_STATE_READY = 0,
    BC_ATSHA204_STATE_READ_SERIAL_NUMBER = 1,
    BC_ATSHA204_STATE_READ_SERIAL_NUMBER2 = 2,
    BC_ATSHA204_STATE_SERIAL_NUMBER = 3,
    BC_ATSHA204_STATE_RANDOM = 4,
    BC_ATSHA204_STATE_READ = 5,
    BC_ATSHA204_STATE_LOCK = 6

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
    uint8_t _rx_buffer[32 + 3];

    bc_atsha204_zone_t _zone;
    uint32_t _address;
    uint8_t *_buffer;
    size_t _max_length;
    size_t _length;
    uint8_t _data_length;
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

//! @brief Lock
//! @param[in] self Instance
//! @param[in] zone Zone
//! @return true On success
//! @return False When other command is in progress

bool bc_atsha204_lock(bc_atsha204_t *self, bc_atsha204_lock_zone_t zone);

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

void bc_atsha204_await(bc_atsha204_t *self);

bool bc_atsha204_random(bc_atsha204_t *self);

bool bc_atsha204_get_random(bc_atsha204_t *self, void *destination, size_t size);

//! @brief Read buffer
//! @param[in] self Instance
//! @param[in] zone Zone
//! @param[in] address EEPROM start address (starts at 0)
//! @param[out] buffer Pointer to destination buffer
//! @param[in] length Number of bytes to be read
//! @return true On success
//! @return false On failure

bool bc_atsha204_read(bc_atsha204_t *self, bc_atsha204_zone_t zone, uint32_t address, void *buffer, size_t length);


//! @}

#endif // _BC_ATSHA204_H

