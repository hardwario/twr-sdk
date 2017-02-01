#ifndef _BC_MODULE_SIGFOX_H
#define _BC_MODULE_SIGFOX_H

#include <bc_td1207r.h>

//! @addtogroup bc_module_sigfox bc_module_sigfox
//! @brief Driver for BigClown SigFox Module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_MODULE_SIGFOX_EVENT_ERROR = BC_TD1207R_EVENT_ERROR,

    //! @brief RF frame transmission started event
    BC_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_START = BC_TD1207R_EVENT_SEND_RF_FRAME_START,

    //! @brief RF frame transmission finished event
    BC_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_DONE = BC_TD1207R_EVENT_SEND_RF_FRAME_DONE

} bc_module_sigfox_event_t;

//! @brief BigClown SigFox Module instance

typedef bc_td1207r_t bc_module_sigfox_t;

//! @brief Initialize BigClown SigFox Module
//! @param[in] self Instance
//! @param[in] reset_signal GPIO channel where RST signal is connected
//! @param[in] uart_channel UART channel where TX and RX signals are connected

void bc_module_sigfox_init(bc_module_sigfox_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_module_sigfox_set_event_handler(bc_module_sigfox_t *self, void (*event_handler)(bc_module_sigfox_t *, bc_module_sigfox_event_t, void *), void *event_param);

//! @brief Check if modem is ready for commands
//! @param[in] self Instance
//! @return true if ready
//! @return false if not ready

bool bc_module_sigfox_is_ready(bc_module_sigfox_t *self);

//! @brief Send RF frame command
//! @param[in] self Instance
//! @param[in] buffer Pointer to data to be transmitted
//! @param[in] length Length of data to be transmitted in bytes (must be from 1 to 12 bytes)
//! @return true if command was accepted for processing
//! @return false if command was denied for processing

bool bc_module_sigfox_send_rf_frame(bc_module_sigfox_t *self, const void *buffer, size_t length);

//! @}

#endif // _BC_MODULE_SIGFOX_H
