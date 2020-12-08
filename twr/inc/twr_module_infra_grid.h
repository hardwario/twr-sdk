#ifndef _TWR_MODULE_INFRA_GRID_H
#define _TWR_MODULE_INFRA_GRID_H

#include <twr_i2c.h>
#include <twr_tca9534a.h>
#include <twr_scheduler.h>

//! @addtogroup twr_module_infra_grid twr_module_infra_grid
//! @brief Library to communicate with Infra Grid Module with Panasonic AMG8833 Grid-EYE sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_MODULE_INFRA_GRID_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_MODULE_INFRA_GRID_EVENT_UPDATE = 1

} twr_module_infra_grid_event_t;

//! @brief Infra Grid Module instance

typedef struct twr_module_infra_grid_t twr_module_infra_grid_t;

//! @brief Infragrid Module Revision

typedef enum
{
    //! @brief Revision 1.0
    TWR_MODULE_INFRA_GRID_REVISION_R1_0 = 0,

    //! @brief Revision 1.1
    TWR_MODULE_INFRA_GRID_REVISION_R1_1 = 1

} twr_module_infra_grid_revision_t;

//! @cond

typedef enum
{
    TWR_MODULE_INFRA_GRID_STATE_ERROR = -1,
    TWR_MODULE_INFRA_GRID_STATE_INITIALIZE = 0,
    TWR_MODULE_INFRA_GRID_STATE_MODE_CHANGE = 1,
    TWR_MODULE_INFRA_GRID_STATE_POWER_UP = 2,
    TWR_MODULE_INFRA_GRID_STATE_INITIAL_RESET = 3,
    TWR_MODULE_INFRA_GRID_STATE_FLAG_RESET = 4,
    TWR_MODULE_INFRA_GRID_STATE_MEASURE = 5,
    TWR_MODULE_INFRA_GRID_STATE_READ = 6,
    TWR_MODULE_INFRA_GRID_STATE_UPDATE = 7

} twr_module_infra_grid_state_t;

struct twr_module_infra_grid_t
{
    int16_t _sensor_data[64];

    twr_tca9534a_t _tca9534;
    twr_module_infra_grid_revision_t _revision;
    twr_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    twr_scheduler_task_id_t _task_id_interval;
    twr_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(twr_module_infra_grid_t *, twr_module_infra_grid_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    twr_tick_t _update_interval;
    twr_module_infra_grid_state_t _state;
    twr_tick_t _tick_ready;
    bool _temperature_valid;

    bool _enable_sleep;
    bool _cmd_sleep;

};

//! @endcond

//! @brief Initialize Infra Grid Module
//! @param[in] self Instance

void twr_module_infra_grid_init(twr_module_infra_grid_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_module_infra_grid_set_event_handler(twr_module_infra_grid_t *self, void (*event_handler)(twr_module_infra_grid_t *, twr_module_infra_grid_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_module_infra_grid_set_update_interval(twr_module_infra_grid_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_module_infra_grid_measure(twr_module_infra_grid_t *self);

//! @brief Read measured data from sensor to the buffer
//! @param[in] self Instance
//! @return true When values are valid
//! @return false When values are invalid

bool twr_module_infra_grid_read_values(twr_module_infra_grid_t *self);

//! @brief Get measured temperature in degrees of Celsius as a array of float numbers
//! @param[in] self Instance
//! @param[out] values Pointer to float array of size 64 where result will be stored

bool twr_module_infra_grid_get_temperatures_celsius(twr_module_infra_grid_t *self, float *values);

//! @brief Read and return thermistor temperature sensor value
//! @param[in] self Instance
//! @return value in degreen of Celsius

float twr_module_infra_grid_read_thermistor(twr_module_infra_grid_t *self);

//! @brief Get module revision
//! @param[in] self Instance
//! @return module revision

twr_module_infra_grid_revision_t twr_module_infra_grid_get_revision(twr_module_infra_grid_t *self);

//! @}

#endif // _TWR_MODULE_INFRA_GRID_H
