#ifndef _BC_PYQ1648_H
#define _BC_PYQ1648_H

#include <bc_tick.h>
#include <bc_gpio.h>
#include <bc_scheduler.h>

//! @addtogroup bc_pyq1648 bc_pyq1648
//! @brief Driver for PYQ1648 PIR sensor
//! @{

//! @brief Callback events

typedef enum
{
    BC_PYQ1648_EVENT_ERROR = 0,
    BC_PYQ1648_EVENT_MOTION = 1

} bc_pyq1648_event_t;

//! @brief Internal states

typedef enum
{
    BC_PYQ1648_STATE_ERROR = -1,
    BC_PYQ1648_STATE_INITIALIZE = 0,
    BC_PYQ1648_STATE_IGNORE = 1,
    BC_PYQ1648_STATE_CHECK = 2

} bc_pyq1648_state_t;

//! @brief Possible sensitivities

typedef enum
{
    BC_PYQ1648_SENSITIVITY_LOW = 0,
    BC_PYQ1648_SENSITIVITY_MEDIUM = 1,
    BC_PYQ1648_SENSITIVITY_HIGH = 2,
    BC_PYQ1648_SENSITIVITY_VERY_HIGH = 3

} bc_pyq1648_sensitivity_t;

//! @brief PYQ1648 instance

typedef struct bc_pyq1648_t bc_pyq1648_t;

//! @cond

struct bc_pyq1648_t
{
    bc_gpio_channel_t _gpio_channel_serin;
    bc_gpio_channel_t _gpio_channel_dl;
    void (*_event_handler)(bc_pyq1648_t *, bc_pyq1648_event_t, void *);
    void *_event_param;
    bc_pyq1648_state_t _state;
    uint32_t _config;
    uint8_t _sensitivity;
    bc_tick_t _blank_period;
    bc_tick_t _aware_time;
    bc_tick_t _ignore_untill;
    bc_scheduler_task_id_t _task_id;
};

//! @endcond

//! @brief Initialize PYQ1648 driver
//! @param[in] self Instance
//! @param[in] gpio_channel_serin GPIO channel for SERIN pin
//! @param[in] gpio_channel_dl GPIO channel for DL pin

void bc_pyq1648_init(bc_pyq1648_t *self, bc_gpio_channel_t gpio_channel_serin, bc_gpio_channel_t gpio_channel_dl);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Callback function
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_pyq1648_set_event_handler(bc_pyq1648_t *self, void (*event_handler)(bc_pyq1648_t *, bc_pyq1648_event_t, void *), void *event_param);

//! @brief Set PYQ1648 sensitivity
//! @param[in] self Instance
//! @param[in] sensitivity sensitivity

void bc_pyq1648_set_sensitivity(bc_pyq1648_t *self, bc_pyq1648_sensitivity_t sensitivity);

//! @brief Set blank period (for how long alarm events will be ignored)
//! @param[in] self Instance
//! @param[in] blank_period Blank period in milliseconds

void bc_pyq1648_set_blank_period(bc_pyq1648_t *self, bc_tick_t blank_period);

//! @}

#endif // _BC_PYQ1648_H
