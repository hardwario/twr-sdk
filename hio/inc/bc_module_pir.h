#ifndef _HIO_MODULE_PIR_H
#define _HIO_MODULE_PIR_H

#include <hio_pyq1648.h>

//! @addtogroup hio_module_pir hio_module_pir
//! @brief Driver for PIR Module
//! @{

//! @brief Motion detection sensitivity

typedef enum
{
    //! @brief Low sensitivity
    HIO_MODULE_PIR_SENSITIVITY_LOW = HIO_PYQ1648_SENSITIVITY_LOW,

    //! @brief Medium sensitivity
    HIO_MODULE_PIR_SENSITIVITY_MEDIUM = HIO_PYQ1648_SENSITIVITY_MEDIUM,

    //! @brief High sensitivity
    HIO_MODULE_PIR_SENSITIVITY_HIGH = HIO_PYQ1648_SENSITIVITY_HIGH,

    //! @brief Very high sensitivity
    HIO_MODULE_PIR_SENSITIVITY_VERY_HIGH = HIO_PYQ1648_SENSITIVITY_VERY_HIGH

} hio_module_pir_sensitivity_t;

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_MODULE_PIR_EVENT_ERROR = HIO_PYQ1648_EVENT_ERROR,

    //! @brief Motion event
    HIO_MODULE_PIR_EVENT_MOTION = HIO_PYQ1648_EVENT_MOTION

} hio_module_pir_event_t;

//! @brief HARDWARIO PIR Module instance

typedef struct hio_pyq1648_t hio_module_pir_t;

//! @brief Initialize PIR Module
//! @param[in] self Instance

void hio_module_pir_init(hio_module_pir_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_module_pir_set_event_handler(hio_module_pir_t *self, void (*event_handler)(hio_module_pir_t *, hio_module_pir_event_t, void*), void *event_param);

//! @brief Set sensor sensitivity
//! @param[in] self Instance
//! @param[in] sensitivity Desired sensitivity

void hio_module_pir_set_sensitivity(hio_module_pir_t *self, hio_module_pir_sensitivity_t sensitivity);
//! @}

#endif // _HIO_MODULE_PIR_H
