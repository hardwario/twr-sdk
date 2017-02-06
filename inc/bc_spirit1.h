#ifndef _BC_SPIRIT1_H
#define _BC_SPIRIT1_H

#include <bc_tick.h>

#define BC_SPIRIT1_MAX_PACKET_SIZE 64

typedef enum
{
    BC_SPIRIT1_EVENT_TX_DONE = 0,
    BC_SPIRIT1_EVENT_RX_DONE = 1,
    BC_SPIRIT1_EVENT_RX_TIMEOUT = 2

} bc_spirit1_event_t;

void bc_spirit1_init(void);

void bc_spirit1_set_event_handler(void (*event_handler)(bc_spirit1_event_t, void *), void *event_param);

void *bc_spirit1_get_tx_buffer(void);

void bc_spirit1_set_tx_length(size_t length);

void *bc_spirit1_get_rx_buffer(void);

size_t bc_spirit1_get_rx_length(void);

void bc_spirit1_set_rx_timeout(bc_tick_t timeout);

void bc_spirit1_tx(void);

void bc_spirit1_rx(void);

void bc_spirit1_sleep(void);

#endif /* _BC_SPIRIT1_H */
