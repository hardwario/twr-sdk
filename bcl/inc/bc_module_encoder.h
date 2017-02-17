#ifndef _BC_MODULE_ENCODER_H
#define _BC_MODULE_ENCODER_H

#include <bc_gpio.h>
#include <bc_button.h>
#include <bc_tick.h>
#include <bc_scheduler.h>

//! @addtogroup bc_module_encoder bc_module_encoder
//! @brief Driver for encoder module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Event button pressed
    BC_MODULE_ENCODER_EVENT_PRESS = 0,

    //! @brief Event button released
    BC_MODULE_ENCODER_EVENT_RELEASE = 1,

    //! @brief Event button clicked (pressed and released within certain time)
    BC_MODULE_ENCODER_EVENT_CLICK = 2,

    //! @brief Event button hold (pressed for longer time)
    BC_MODULE_ENCODER_EVENT_HOLD = 3,

    //! @brief Event encoder up
    BC_MODULE_ENCODER_EVENT_UP = 4,

    //! @brief Event encoder down
    BC_MODULE_ENCODER_EVENT_DOWN = 5

} bc_module_encoder_event_t;

//! @brief Encoder Module instance

typedef struct bc_module_encoder_t bc_module_encoder_t;

//! @cond

struct bc_module_encoder_t
{
    bc_button_t _button;
    uint8_t _encoder_last_state;
    void (*_event_handler)(bc_module_encoder_event_t, void *);
    void *_event_param;

    bc_scheduler_task_id_t task_id;

    int32_t value;
    int32_t eventValue;
};

//! @endcond

//! @brief Initialize encoder module

void bc_module_encoder_init();

//! @brief Set callback function
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_module_encoder_set_event_handler(void (*event_handler)(bc_module_encoder_event_t, void *), void *event_param);

//! @}

#endif // _BC_MODULE_ENCODER_H
