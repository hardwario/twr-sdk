#ifndef _BC_IR_RX_H
#define _BC_IR_RX_H

#include <bc_common.h>

typedef enum
{
    //! @brief Receiver IR command
    BC_IR_RX_NEC_FORMAT = 0,

    //! @brief Received repeat command
    BC_IR_RX_NEC_FORMAT_REPEAT = 1

} bc_ir_rx_event_t;

//! @brief Init infrared RX driver

void bc_ir_rx_init();

//! @brief Set callback function
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_ir_rx_set_event_handler(void (*event_handler)(bc_ir_rx_event_t, void *), void *event_param);

//! @brief Get received code

void bc_ir_rx_get_code(uint32_t *nec_code);

#endif //_BC_IR_RX_H
