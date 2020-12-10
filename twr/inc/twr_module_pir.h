#ifndef _TWR_MODULE_PIR_H
#define _TWR_MODULE_PIR_H

#include <twr_pyq1648.h>

//! @addtogroup twr_module_pir twr_module_pir
//! @brief Driver for PIR Module
//! @{

//! @brief Motion detection sensitivity

typedef enum
{
    //! @brief Low sensitivity
    TWR_MODULE_PIR_SENSITIVITY_LOW = TWR_PYQ1648_SENSITIVITY_LOW,

    //! @brief Medium sensitivity
    TWR_MODULE_PIR_SENSITIVITY_MEDIUM = TWR_PYQ1648_SENSITIVITY_MEDIUM,

    //! @brief High sensitivity
    TWR_MODULE_PIR_SENSITIVITY_HIGH = TWR_PYQ1648_SENSITIVITY_HIGH,

    //! @brief Very high sensitivity
    TWR_MODULE_PIR_SENSITIVITY_VERY_HIGH = TWR_PYQ1648_SENSITIVITY_VERY_HIGH

} twr_module_pir_sensitivity_t;

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_MODULE_PIR_EVENT_ERROR = TWR_PYQ1648_EVENT_ERROR,

    //! @brief Motion event
    TWR_MODULE_PIR_EVENT_MOTION = TWR_PYQ1648_EVENT_MOTION

} twr_module_pir_event_t;

//! @brief HARDWARIO PIR Module instance

typedef struct twr_pyq1648_t twr_module_pir_t;

//! @brief Initialize PIR Module
//! @param[in] self Instance

void twr_module_pir_init(twr_module_pir_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_module_pir_set_event_handler(twr_module_pir_t *self, void (*event_handler)(twr_module_pir_t *, twr_module_pir_event_t, void*), void *event_param);

//! @brief Set sensor sensitivity
//! @param[in] self Instance
//! @param[in] sensitivity Desired sensitivity

void twr_module_pir_set_sensitivity(twr_module_pir_t *self, twr_module_pir_sensitivity_t sensitivity);
//! @}

#endif // _TWR_MODULE_PIR_H
