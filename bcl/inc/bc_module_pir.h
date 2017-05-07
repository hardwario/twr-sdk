#ifndef _BC_MODULE_PIR_H
#define _BC_MODULE_PIR_H

#include <bc_pyq1648.h>

//! @addtogroup bc_module_pir bc_module_pir
//! @brief Driver for PIR Module
//! @{

//! @brief Motion detection sensitivity

typedef enum
{
    //! @brief Low sensitivity
    BC_MODULE_PIR_SENSITIVITY_LOW = BC_PYQ1648_SENSITIVITY_LOW,

    //! @brief Medium sensitivity
    BC_MODULE_PIR_SENSITIVITY_MEDIUM = BC_PYQ1648_SENSITIVITY_MEDIUM,

    //! @brief High sensitivity
    BC_MODULE_PIR_SENSITIVITY_HIGH = BC_PYQ1648_SENSITIVITY_HIGH,

    //! @brief Very high sensitivity
    BC_MODULE_PIR_SENSITIVITY_VERY_HIGH = BC_PYQ1648_SENSITIVITY_VERY_HIGH

} bc_module_pir_sensitivity_t;

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_MODULE_PIR_EVENT_ERROR = BC_PYQ1648_EVENT_ERROR,

    //! @brief Motion event
    BC_MODULE_PIR_EVENT_MOTION = BC_PYQ1648_EVENT_MOTION

} bc_module_pir_event_t;

//! @brief BigClown PIR Module instance

typedef struct bc_pyq1648_t bc_module_pir_t;

//! @brief Initialize PIR Module
//! @param[in] self Instance

void bc_module_pir_init(bc_module_pir_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_module_pir_set_event_handler(bc_module_pir_t *self, void (*event_handler)(bc_module_pir_t *, bc_module_pir_event_t, void*), void *event_param);

//! @brief Set sensor sensitivity
//! @param[in] self Instance
//! @param[in] sensitivity Desired sensitivity

void bc_module_pir_set_sensitivity(bc_module_pir_t *self, bc_module_pir_sensitivity_t sensitivity);
//! @}

#endif // _BC_MODULE_PIR_H
