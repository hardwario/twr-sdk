#ifndef _HIO_IR_RX_H
#define _HIO_IR_RX_H

#include <hio_common.h>

typedef enum
{
    //! @brief Receiver IR command
    HIO_IR_RX_NEC_FORMAT = 0,

    //! @brief Received repeat command
    HIO_IR_RX_NEC_FORMAT_REPEAT = 1

} hio_ir_rx_event_t;

//! @brief Init infrared RX driver

void hio_ir_rx_init();

//! @brief Set callback function
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_ir_rx_set_event_handler(void (*event_handler)(hio_ir_rx_event_t, void *), void *event_param);

//! @brief Get received code

void hio_ir_rx_get_code(uint32_t *nec_code);

#endif //_HIO_IR_RX_H
