#ifndef _HIO_MODULE_ENCODER_H
#define _HIO_MODULE_ENCODER_H

#include <hio_button.h>

//! @addtogroup hio_module_encoder hio_module_encoder
//! @brief Driver for Encoder Module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Event rotation
    HIO_MODULE_ENCODER_EVENT_ROTATION = 0,

    //! @brief Event button pressed
    HIO_MODULE_ENCODER_EVENT_PRESS = 1,

    //! @brief Event button released
    HIO_MODULE_ENCODER_EVENT_RELEASE = 2,

    //! @brief Event button clicked (pressed and released within certain time)
    HIO_MODULE_ENCODER_EVENT_CLICK = 3,

    //! @brief Event button hold (pressed for longer time)
    HIO_MODULE_ENCODER_EVENT_HOLD = 4,

    //! @brief Event error (module is not present)
    HIO_MODULE_ENCODER_EVENT_ERROR = 5


} hio_module_encoder_event_t;

//! @brief Initialize Encoder Module

void hio_module_encoder_init(void);

//! @brief Deinitialize Encoder Module

void hio_module_encoder_deinit(void);

//! @brief Set callback function
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_module_encoder_set_event_handler(void (*event_handler)(hio_module_encoder_event_t, void *), void *event_param);

//! @brief Get encoder button instance
//! @return Pointer to button instance

hio_button_t *hio_module_encoder_get_button_instance(void);

//! @brief Read encoder delta increment (can be positive or negative number)
//! @return Delta increment

int hio_module_encoder_get_increment(void);

//! @brief Get Encoder Module is pressent, can use without hio_module_encoder_init

bool hio_module_encoder_is_present(void);

//! @}

#endif // _HIO_MODULE_ENCODER_H
