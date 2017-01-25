#ifndef _BC_SPIRIT1_H
#define _BC_SPIRIT1_H

#include <bc_tick.h>

#define BC_SPIRIT1_MAX_PACKET_SIZE 64

typedef enum
{
    BC_SPIRIT1_EVENT_TRANSMISSION_DONE = 0,
    BC_SPIRIT1_EVENT_RECEPTION_DONE = 1,
    BC_SPIRIT1_EVENT_RECEPTION_TIMEOUT = 2

} bc_spirit1_event_t;

void bc_spirit1_init(void);

void bc_spirit1_set_event_handler(void (*event_handler)(bc_spirit1_event_t));

void bc_spirit1_transmit(const void *buffer, size_t length);

void bc_spirit1_receive(void *buffer, size_t *length, bc_tick_t timeout);

void bc_spirit1_sleep(void);

#endif /* _BC_SPIRIT1_H */
