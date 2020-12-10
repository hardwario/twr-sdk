#ifndef _TWR_SPIRIT1_H
#define _TWR_SPIRIT1_H

#include <twr_tick.h>

//! @addtogroup twr_spirit twr_spirit
//! @brief Driver for spirit RF transceiver module
//! @{

//! @cond

#define TWR_SPIRIT1_MAX_PACKET_SIZE 64

//! @endcond

//! @brief Callback events

typedef enum
{
    //! @brief Event is TX done
    TWR_SPIRIT1_EVENT_TX_DONE = 0,

    //! @brief Event is RX done
    TWR_SPIRIT1_EVENT_RX_DONE = 1,

    //! @brief Event is RX timeout
    TWR_SPIRIT1_EVENT_RX_TIMEOUT = 2

} twr_spirit1_event_t;

//! @brief Initialize
//! @return true On success
//! @return false On failure

bool twr_spirit1_init(void);

//! @brief Deitialize
//! @return true On success
//! @return false On failure

bool twr_spirit1_deinit(void);

//! @brief Set callback function
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_spirit1_set_event_handler(void (*event_handler)(twr_spirit1_event_t, void *), void *event_param);

//! @brief Get TX buffer
//! @return Pointer to buffer

void *twr_spirit1_get_tx_buffer(void);

//! @brief Set TX buffer length
//! @param[in] length TX buffer length

void twr_spirit1_set_tx_length(size_t length);

//! @brief Get TX buffer length
//! @return Size of buffer

size_t twr_spirit1_get_tx_length(void);

//! @brief Get RX buffer
//! @return Pointer to buffer

void *twr_spirit1_get_rx_buffer(void);

//! @brief Get RX buffer length
//! @return Size of buffer

size_t twr_spirit1_get_rx_length(void);

//! @brief Get RSSI
//! @return RSSI

int twr_spirit1_get_rx_rssi(void);

//! @brief Set TX timeout
//! @param[in] timeout Maximum timeout for receiving

void twr_spirit1_set_rx_timeout(twr_tick_t timeout);

//! @brief Enter TX state

void twr_spirit1_tx(void);

//! @brief Enter RX state

void twr_spirit1_rx(void);

//! @brief Enter sleep state

void twr_spirit1_sleep(void);

//! @}

#endif // _TWR_SPIRIT1_H
