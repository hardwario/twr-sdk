#ifndef INC_BC_PYQ1648_H_
#define INC_BC_PYQ1648_H_

#include <bc_common.h>
#include <bc_tick.h>
#include <bc_gpio.h>

//! @addtogroup bc_pyq1648 bc_pyq1648
//! @brief Driver for pyq1648 pir sensor
//! @{

typedef enum
{
    BC_PYQ1648_EVENT_ERROR = 0,
    BC_PYQ1648_EVENT_MOTION = 1

} bc_pyq1648_event_t;

typedef enum
{
    BC_PYQ1648_STATE_ERROR = -1,
    BC_PYQ1648_STATE_INITIALIZE = 0,
    BC_PYQ1648_STATE_CHECK = 1

} bc_pyq1648_state_t;

typedef enum
{
    BC_PYQ1648_SENSITIVITY_LOW = 0,
    BC_PYQ1648_SENSITIVITY_MEDIUM,
    BC_PYQ1648_SENSITIVITY_HIGH
} bc_pyq1648_sensitivity_t;

typedef struct bc_pyq1648_t bc_pyq1648_t;

struct bc_pyq1648_t
{
    bc_gpio_channel_t _gpio_channel_serin;
    bc_gpio_channel_t _gpio_channel_dl;
    void (*_event_handler)(bc_pyq1648_t *, bc_pyq1648_event_t);
    bc_tick_t _update_interval;
    bc_tick_t _aware_time;
    bc_pyq1648_state_t _state;
    bool _event_valid;
    uint8_t _sensitivity;
    bc_tick_t _blank_period;
    uint32_t _config;
};

//! @brief Initialize pyq1648 on channels gpio_channel_serin and gpio_channel_dl
//! @param[in] self PIR image
//! @param[in] gpio_channel_serin PIR serin channel
//! @param[in] gpio_channel_dl PIR dl channel

void bc_pyq1648_init(bc_pyq1648_t *self, bc_gpio_channel_t gpio_channel_serin, bc_gpio_channel_t gpio_channel_dl);

//! @brief Set pyq1648 event handler
//! @param[in] self pyq1648 image
//! @param[in] event_handler pyq1648 event handler

void bc_pyq1648_set_event_handler(bc_pyq1648_t *self, void (*event_handler)(bc_pyq1648_t *, bc_pyq1648_event_t));

//! @brief Set pyq1648 set sensitivity
//! @param[in] self pyq1648 image
//! @param[in] event_handler pyq1648 event handler

void bc_pyq1648_set_sensitivity(bc_pyq1648_t *self, bc_pyq1648_sensitivity_t sensitivity);

//! @}

#endif /* INC_BC_PYQ1648_H_ */
