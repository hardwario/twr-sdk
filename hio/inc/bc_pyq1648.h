#ifndef _HIO_PYQ1648_H
#define _HIO_PYQ1648_H

#include <hio_tick.h>
#include <hio_gpio.h>
#include <hio_scheduler.h>

//! @addtogroup hio_pyq1648 hio_pyq1648
//! @brief Driver for PYQ1648 PIR sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_PYQ1648_EVENT_ERROR = 0,

    //! @brief Update event
    HIO_PYQ1648_EVENT_MOTION = 1

} hio_pyq1648_event_t;

//! @brief Possible sensitivities

typedef enum
{
    //! @brief Low sensitivity
    HIO_PYQ1648_SENSITIVITY_LOW = 0,

    //! @brief Medium sensitivity
    HIO_PYQ1648_SENSITIVITY_MEDIUM = 1,

    //! @brief High sensitivity
    HIO_PYQ1648_SENSITIVITY_HIGH = 2,

    //! @brief Very high sensitivity
    HIO_PYQ1648_SENSITIVITY_VERY_HIGH = 3

} hio_pyq1648_sensitivity_t;

//! @brief PYQ1648 instance

typedef struct hio_pyq1648_t hio_pyq1648_t;

//! @cond

typedef enum
{
    HIO_PYQ1648_STATE_ERROR = -1,
    HIO_PYQ1648_STATE_INITIALIZE = 0,
    HIO_PYQ1648_STATE_IGNORE = 1,
    HIO_PYQ1648_STATE_CHECK = 2

} hio_pyq1648_state_t;

struct hio_pyq1648_t
{
    hio_gpio_channel_t _gpio_channel_serin;
    hio_gpio_channel_t _gpio_channel_dl;
    void (*_event_handler)(hio_pyq1648_t *, hio_pyq1648_event_t, void *);
    void *_event_param;
    hio_pyq1648_state_t _state;
    uint32_t _config;
    uint8_t _sensitivity;
    hio_tick_t _blank_period;
    hio_tick_t _aware_time;
    hio_tick_t _ignore_untill;
    hio_scheduler_task_id_t _task_id;
};

//! @endcond

//! @brief Initialize PYQ1648 driver
//! @param[in] self Instance
//! @param[in] gpio_channel_serin GPIO channel for SERIN pin
//! @param[in] gpio_channel_dl GPIO channel for DL pin

void hio_pyq1648_init(hio_pyq1648_t *self, hio_gpio_channel_t gpio_channel_serin, hio_gpio_channel_t gpio_channel_dl);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Callback function
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_pyq1648_set_event_handler(hio_pyq1648_t *self, void (*event_handler)(hio_pyq1648_t *, hio_pyq1648_event_t, void *), void *event_param);

//! @brief Set PYQ1648 sensitivity
//! @param[in] self Instance
//! @param[in] sensitivity Sensitivity

void hio_pyq1648_set_sensitivity(hio_pyq1648_t *self, hio_pyq1648_sensitivity_t sensitivity);

//! @brief Set blank period (for how long alarm events will be ignored)
//! @param[in] self Instance
//! @param[in] blank_period Blank period in milliseconds

void hio_pyq1648_set_blank_period(hio_pyq1648_t *self, hio_tick_t blank_period);

//! @}

#endif // _HIO_PYQ1648_H
