#ifndef _BC_MODULE_ENCODER_H
#define _BC_MODULE_ENCODER_H

#include <bc_button.h>

//! @addtogroup bc_module_encoder bc_module_encoder
//! @brief Driver for Encoder Module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Update event
    BC_MODULE_ENCODER_EVENT_UPDATE = 0,

} bc_module_encoder_event_t;

//! @brief Initialize Encoder Module

void bc_module_encoder_init(void);

//! @brief Set callback function
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_module_encoder_set_event_handler(void (*event_handler)(bc_module_encoder_event_t, void *), void *event_param);

//! @brief Get encoder button instance
//! @return Pointer to button instance

bc_button_t *bc_module_encoder_get_button_instance(void);

//! @brief Read encoder delta increment (can be positive or negative number)
//! @return Delta increment

int bc_module_encoder_get_increment(void);

//! @}

#endif // _BC_MODULE_ENCODER_H
