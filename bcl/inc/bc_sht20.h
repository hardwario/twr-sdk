#ifndef _BC_SHT20_H
#define _BC_SHT20_H

#include <bc_i2c.h>
#include <bc_scheduler.h>

//! @addtogroup bc_sht20 bc_sht20
//! @brief Driver for SHT20 humidity sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_SHT20_EVENT_ERROR = 0,

    //! @brief Update event
    BC_SHT20_EVENT_UPDATE = 1

} bc_sht20_event_t;

//! @brief SHT20 instance

typedef struct bc_sht20_t bc_sht20_t;

//! @cond

typedef enum
{
    BC_SHT20_STATE_ERROR = -1,
    BC_SHT20_STATE_INITIALIZE = 0,
    BC_SHT20_STATE_MEASURE_RH = 1,
    BC_SHT20_STATE_READ_RH = 2,
    BC_SHT20_STATE_MEASURE_T = 3,
    BC_SHT20_STATE_READ_T = 4,
    BC_SHT20_STATE_UPDATE = 5

} bc_sht20_state_t;

struct bc_sht20_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(bc_sht20_t *, bc_sht20_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    bc_tick_t _update_interval;
    bc_sht20_state_t _state;
    bc_tick_t _tick_ready;
    bool _humidity_valid;
    bool _temperature_valid;
    uint16_t _reg_humidity;
    uint16_t _reg_temperature;
};

//! @endcond

//! @brief Initialize SHT20
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void bc_sht20_init(bc_sht20_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_sht20_set_event_handler(bc_sht20_t *self, void (*event_handler)(bc_sht20_t *, bc_sht20_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_sht20_set_update_interval(bc_sht20_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_sht20_measure(bc_sht20_t *self);

//! @brief Get measured humidity as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_sht20_get_humidity_raw(bc_sht20_t *self, uint16_t *raw);

//! @brief Get measured humidity as percentage
//! @param[in] self Instance
//! @param[in] percentage Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_sht20_get_humidity_percentage(bc_sht20_t *self, float *percentage);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_sht20_get_temperature_raw(bc_sht20_t *self, uint16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_sht20_get_temperature_celsius(bc_sht20_t *self, float *celsius);

//! @}

#endif // _BC_SHT20_H
