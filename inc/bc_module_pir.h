#ifndef _BC_MODULE_PIR_H
#define _BC_MODULE_PIR_H

#include <bc_pyq1648.h>

//! @addtogroup bc_module_pir bc_module_pir
//! @brief Driver for PIR module
//! @{

typedef bc_pyq1648_sensitivity_t bc_module_pir_sensitivity_t;
typedef bc_pyq1648_event_t bc_module_pir_event_t;
typedef bc_pyq1648_t bc_module_pir_t;

//! @brief Initialize PIR Module
//! @param[in] self PIR image

void bc_module_pir_init(bc_module_pir_t *self);

//! @brief Set PIR Module event handler
//! @param[in] self PIR Module image
//! @param[in] event_handler PIR Module event handler
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_module_pir_set_event_handler(bc_module_pir_t *self, void (*event_handler)(bc_module_pir_t *, bc_module_pir_event_t, void*), void *event_param);

//! @brief Set PIR Module sensitivity
//! @param[in] self PIR Module image
//! @param[in] sensitivity PIR Module sensitivity

void bc_module_pir_set_sensitivity(bc_pyq1648_t *self, bc_module_pir_sensitivity_t sensitivity);
//! @}

#endif // _BC_MODULE_PIR_H
