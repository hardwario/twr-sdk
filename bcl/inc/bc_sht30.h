#ifndef _BC_SHT30_H
#define _BC_SHT30_H

#include <bc_i2c.h>
#include <bc_scheduler.h>

//! @addtogroup bc_sht30 bc_sht30
//! @brief Driver for SHT30 humidity sensor
//! @{

#define BC_SHT30_ADDRESS_DEFAULT 0x44
#define BC_SHT30_ADDRESS_ALTERNATE 0x45

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_SHT30_EVENT_ERROR = 0,

    //! @brief Update event
    BC_SHT30_EVENT_UPDATE = 1

} bc_sht30_event_t;

//! @brief SHT30 instance

typedef struct bc_sht30_t bc_sht30_t;

//! @cond

typedef enum
{
    BC_SHT30_STATE_ERROR = -1,
    BC_SHT30_STATE_INITIALIZE = 0,
    BC_SHT30_STATE_MEASURE = 1,
    BC_SHT30_STATE_READ = 2,
    BC_SHT30_STATE_UPDATE = 3

} bc_sht30_state_t;

struct bc_sht30_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(bc_sht30_t *, bc_sht30_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    bc_tick_t _update_interval;
    bc_sht30_state_t _state;
    bc_tick_t _tick_ready;
    bool _humidity_valid;
    bool _temperature_valid;
    uint16_t _reg_humidity;
    uint16_t _reg_temperature;
};

//! @endcond

//! @brief Initialize SHT30
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void bc_sht30_init(bc_sht30_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_sht30_set_event_handler(bc_sht30_t *self, void (*event_handler)(bc_sht30_t *, bc_sht30_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_sht30_set_update_interval(bc_sht30_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_sht30_measure(bc_sht30_t *self);

//! @brief Get measured humidity as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_sht30_get_humidity_raw(bc_sht30_t *self, uint16_t *raw);

//! @brief Get measured humidity as percentage
//! @param[in] self Instance
//! @param[in] percentage Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_sht30_get_humidity_percentage(bc_sht30_t *self, float *percentage);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_sht30_get_temperature_raw(bc_sht30_t *self, uint16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_sht30_get_temperature_celsius(bc_sht30_t *self, float *celsius);

//! @}

#endif // _BC_SHT30_H
