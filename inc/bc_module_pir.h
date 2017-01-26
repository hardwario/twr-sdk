#ifndef _BC_MODULE_PIR_H
#define _BC_MODULE_PIR_H

#include <bc_pyq1648.h>

//! @addtogroup bc_module_pir bc_module_pir
//! @brief Driver for PIR module
//! @{

typedef bc_pyq1648_sensitivity_t bc_module_pir_sensitivity_t;
typedef bc_pyq1648_event_t bc_module_pir_event_t;
typedef bc_pyq1648_t bc_module_pir_t;

// TODO Delete this block...

/*
 #define bc_module_pir_init(_SELF_, _GPIO_CHANNEL_SETUP_, _GPIO_CHANNEL_EVENT_) (bc_pyq1648_init((_SELF_), (_GPIO_CHANNEL_), (_GPIO_CHANNEL_EVENT_)))
 #define bc_module_pir_set_event_handler(_SELF_, _HANDLER_) (bc_pyq1648_set_event_handler((_SELF_), void (*event_handler)(_HANDLER_)))
 #define bc_module_pir_set_update_interval(_SELF_, _INTERVAL_) (bc_pyq1648_set_update_interval((_SELF_), (_INTERVAL_)));
 */

//! @brief Initialize PIR module on channels gpio_channel_setup and gpio_channel_event
//! @param[in] self PIR image
//! @param[in] gpio_channel_setup PIR setup channel
//! @param[in] gpio_channel_event PIR event channel

// TODO Delete there should be no GPIO configuration - this is fixed by hardware

inline void bc_module_pir_init(bc_module_pir_t *self, bc_gpio_channel_t gpio_channel_setup, bc_gpio_channel_t gpio_channel_event)
{
    bc_pyq1648_init(self, gpio_channel_setup, gpio_channel_event);
}

//! @brief Set PIR module event handler
//! @param[in] self PIR image
//! @param[in] event_handler PIR event handler

inline void bc_module_pir_set_event_handler(bc_module_pir_t *self, void (*event_handler)(bc_module_pir_t *, bc_module_pir_event_t))
{
    bc_pyq1648_set_event_handler(self, event_handler);
}

//! @brief Set PIR module set sensitivity
//! @param[in] self PIR module image
//! @param[in] event_handler PIR module event handler

inline void bc_module_pir_set_sensitivity(bc_pyq1648_t *self, bc_module_pir_sensitivity_t sensitivity)
{
    bc_pyq1648_set_sensitivity(self, sensitivity);
}

//! @}

#endif // _BC_MODULE_PIR_H
