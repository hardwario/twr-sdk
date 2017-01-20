#ifndef INC_BC_PIR_MODULE_H_
#define INC_BC_PIR_MODULE_H_

#include <bc_pyq1648.h>

//! @addtogroup bc_pir_module bc_pir_module
//! @brief Driver for PIR module
//! @{

typedef bc_pyq1648_sensitivity_t bc_pir_module_sensitivity_t;
typedef bc_pyq1648_event_t bc_pir_module_event_t;
typedef bc_pyq1648_t bc_pir_module_t;

/*
 #define bc_pir_module_init(_SELF_, _GPIO_CHANNEL_SETUP_, _GPIO_CHANNEL_EVENT_) (bc_pyq1648_init((_SELF_), (_GPIO_CHANNEL_), (_GPIO_CHANNEL_EVENT_)))
 #define bc_pir_module_set_event_handler(_SELF_, _HANDLER_) (bc_pyq1648_set_event_handler((_SELF_), void (*event_handler)(_HANDLER_)))
 #define bc_pir_module_set_update_interval(_SELF_, _INTERVAL_) (bc_pyq1648_set_update_interval((_SELF_), (_INTERVAL_)));
 */

//! @brief Initialize PIR module on channels gpio_channel_setup and gpio_channel_event
//! @param[in] self PIR image
//! @param[in] gpio_channel_setup PIR setup channel
//! @param[in] gpio_channel_event PIR event channel
inline void bc_pir_module_init(bc_pir_module_t *self, bc_gpio_channel_t gpio_channel_setup, bc_gpio_channel_t gpio_channel_event)
{
    bc_pyq1648_init(self, gpio_channel_setup, gpio_channel_event);
}

//! @brief Set PIR module event handler
//! @param[in] self PIR image
//! @param[in] event_handler PIR event handler

inline void bc_pir_module_set_event_handler(bc_pir_module_t *self, void (*event_handler)(bc_pir_module_t *, bc_pir_module_event_t))
{
    bc_pyq1648_set_event_handler(self, event_handler);
}

//! @brief Set PIR module set sensitivity
//! @param[in] self PIR module image
//! @param[in] event_handler PIR module event handler

inline void bc_pir_module_set_sensitivity(bc_pyq1648_t *self, bc_pir_module_sensitivity_t sensitivity)
{
    bc_pyq1648_set_sensitivity(self, sensitivity);
}

//! @}

#endif /* INC_BC_PIR_MODULE_H_ */
