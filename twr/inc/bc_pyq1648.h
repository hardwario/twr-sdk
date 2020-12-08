#ifndef _TWR_PYQ1648_H
#define _TWR_PYQ1648_H

#include <twr_tick.h>
#include <twr_gpio.h>
#include <twr_scheduler.h>

//! @addtogroup twr_pyq1648 twr_pyq1648
//! @brief Driver for PYQ1648 PIR sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_PYQ1648_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_PYQ1648_EVENT_MOTION = 1

} twr_pyq1648_event_t;

//! @brief Possible sensitivities

typedef enum
{
    //! @brief Low sensitivity
    TWR_PYQ1648_SENSITIVITY_LOW = 0,

    //! @brief Medium sensitivity
    TWR_PYQ1648_SENSITIVITY_MEDIUM = 1,

    //! @brief High sensitivity
    TWR_PYQ1648_SENSITIVITY_HIGH = 2,

    //! @brief Very high sensitivity
    TWR_PYQ1648_SENSITIVITY_VERY_HIGH = 3

} twr_pyq1648_sensitivity_t;

//! @brief PYQ1648 instance

typedef struct twr_pyq1648_t twr_pyq1648_t;

//! @cond

typedef enum
{
    TWR_PYQ1648_STATE_ERROR = -1,
    TWR_PYQ1648_STATE_INITIALIZE = 0,
    TWR_PYQ1648_STATE_IGNORE = 1,
    TWR_PYQ1648_STATE_CHECK = 2

} twr_pyq1648_state_t;

struct twr_pyq1648_t
{
    twr_gpio_channel_t _gpio_channel_serin;
    twr_gpio_channel_t _gpio_channel_dl;
    void (*_event_handler)(twr_pyq1648_t *, twr_pyq1648_event_t, void *);
    void *_event_param;
    twr_pyq1648_state_t _state;
    uint32_t _config;
    uint8_t _sensitivity;
    twr_tick_t _blank_period;
    twr_tick_t _aware_time;
    twr_tick_t _ignore_untill;
    twr_scheduler_task_id_t _task_id;
};

//! @endcond

//! @brief Initialize PYQ1648 driver
//! @param[in] self Instance
//! @param[in] gpio_channel_serin GPIO channel for SERIN pin
//! @param[in] gpio_channel_dl GPIO channel for DL pin

void twr_pyq1648_init(twr_pyq1648_t *self, twr_gpio_channel_t gpio_channel_serin, twr_gpio_channel_t gpio_channel_dl);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Callback function
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_pyq1648_set_event_handler(twr_pyq1648_t *self, void (*event_handler)(twr_pyq1648_t *, twr_pyq1648_event_t, void *), void *event_param);

//! @brief Set PYQ1648 sensitivity
//! @param[in] self Instance
//! @param[in] sensitivity Sensitivity

void twr_pyq1648_set_sensitivity(twr_pyq1648_t *self, twr_pyq1648_sensitivity_t sensitivity);

//! @brief Set blank period (for how long alarm events will be ignored)
//! @param[in] self Instance
//! @param[in] blank_period Blank period in milliseconds

void twr_pyq1648_set_blank_period(twr_pyq1648_t *self, twr_tick_t blank_period);

//! @}

#endif // _TWR_PYQ1648_H
