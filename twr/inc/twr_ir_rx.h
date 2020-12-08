#ifndef _TWR_IR_RX_H
#define _TWR_IR_RX_H

#include <twr_common.h>

typedef enum
{
    //! @brief Receiver IR command
    TWR_IR_RX_NEC_FORMAT = 0,

    //! @brief Received repeat command
    TWR_IR_RX_NEC_FORMAT_REPEAT = 1

} twr_ir_rx_event_t;

//! @brief Init infrared RX driver

void twr_ir_rx_init();

//! @brief Set callback function
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_ir_rx_set_event_handler(void (*event_handler)(twr_ir_rx_event_t, void *), void *event_param);

//! @brief Get received code

void twr_ir_rx_get_code(uint32_t *nec_code);

#endif //_TWR_IR_RX_H
