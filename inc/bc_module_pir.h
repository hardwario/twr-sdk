#ifndef _BC_MODULE_PIR_H
#define _BC_MODULE_PIR_H

#include <bc_pyq1648.h>

//! @addtogroup bc_module_pir bc_module_pir
//! @brief Driver for PIR module
//! @{

typedef bc_pyq1648_sensitivity_t bc_module_pir_sensitivity_t;
typedef bc_pyq1648_event_t bc_module_pir_event_t;
typedef bc_pyq1648_t bc_module_pir_t;

//! @brief Initialize PIR module on channels gpio_channel_setup and gpio_channel_event
//! @param[in] self PIR image
//! @param[in] gpio_channel_setup PIR setup channel
//! @param[in] gpio_channel_event PIR event channel

void bc_module_pir_init(bc_module_pir_t *self);

//! @brief Set PIR module event handler
//! @param[in] self PIR image
//! @param[in] event_handler PIR event handler

void bc_module_pir_set_event_handler(bc_module_pir_t *self, void (*event_handler)(bc_module_pir_t *, bc_module_pir_event_t, void*), void *event_param);

//! @brief Set PIR module set sensitivity
//! @param[in] self PIR module image
//! @param[in] event_handler PIR module event handler

void bc_module_pir_set_sensitivity(bc_pyq1648_t *self, bc_module_pir_sensitivity_t sensitivity);
//! @}

#endif // _BC_MODULE_PIR_H
