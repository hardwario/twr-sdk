#ifndef _BC_MODULE_INFRA_GRID_H
#define _BC_MODULE_INFRA_GRID_H

#include <bc_i2c.h>
#include <bc_scheduler.h>

//! @addtogroup bc_module_infra_grid bc_module_infra_grid
//! @brief Library to communicate with Infra Grid Module with Panasonic AMG8833 Grid-EYE sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_MODULE_INFRA_GRID_EVENT_ERROR = 0,

    //! @brief Update event
    BC_MODULE_INFRA_GRID_EVENT_UPDATE = 1

} bc_module_infra_grid_event_t;

//! @brief Infra Grid Module instance

typedef struct bc_module_infra_grid_t bc_module_infra_grid_t;

//! @cond

typedef enum
{
    BC_MODULE_INFRA_GRID_STATE_ERROR = -1,
    BC_MODULE_INFRA_GRID_STATE_INITIALIZE = 0,
    BC_MODULE_INFRA_GRID_STATE_MODE_CHANGE = 1,
    BC_MODULE_INFRA_GRID_STATE_INITIAL_RESET = 2,
    BC_MODULE_INFRA_GRID_STATE_FLAG_RESET = 3,
    BC_MODULE_INFRA_GRID_STATE_MEASURE = 4,
    BC_MODULE_INFRA_GRID_STATE_READ = 5,
    BC_MODULE_INFRA_GRID_STATE_UPDATE = 6

} bc_module_infra_grid_state_t;

struct bc_module_infra_grid_t
{
    int16_t _sensor_data[64];

    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(bc_module_infra_grid_t *, bc_module_infra_grid_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    bc_tick_t _update_interval;
    bc_module_infra_grid_state_t _state;
    bc_tick_t _tick_ready;
    bool _temperature_valid;

    bool _enable_sleep;
    bool _cmd_sleep;

};

//! @endcond

//! @brief Initialize Infra Grid Module
//! @param[in] self Instance

void bc_module_infra_grid_init(bc_module_infra_grid_t *self);

void bc_module_infra_grid_set_event_handler(bc_module_infra_grid_t *self, void (*event_handler)(bc_module_infra_grid_t *, bc_module_infra_grid_event_t, void *), void *event_param);

void bc_module_infra_grid_set_update_interval(bc_module_infra_grid_t *self, bc_tick_t interval);

bool bc_module_infra_grid_measure(bc_module_infra_grid_t *self);

bool bc_module_ifra_grid_read_values(bc_module_infra_grid_t *self);

void bc_module_ifra_grid_get_temperatures_celsius(bc_module_infra_grid_t *self, float *values);

float bc_module_ifra_grid_read_thermistor(bc_module_infra_grid_t *self);

//void bc_module_infra_grid_set_update_interval(bc_module_infra_grid_t *self, bc_tick_t interval);
//void bc_module_infra_grid_set_event_handler(bc_module_infra_grid_t *self, void (*event_handler)(bc_module_infra_grid_event_t, void *), void *event_param);

//! @}

#endif // _BC_MODULE_INFRA_GRID_H
