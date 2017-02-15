#ifndef _BC_RADIO_H
#define _BC_RADIO_H

#include <bc_common.h>

typedef enum
{
    BC_RADIO_EVENT_PAIR_SUCCESS = 0,
    BC_RADIO_EVENT_PAIR_FAILURE = 1

} bc_radio_event_t;

void bc_radio_init(void);

void bc_radio_set_event_handler(void (*event_handler)(bc_radio_event_t, void *), void *event_param);

void bc_radio_listen(void);

void bc_radio_sleep(void);

void bc_radio_enroll_to_gateway(void);

void bc_radio_enrollment_start(void);

void bc_radio_enrollment_stop(void);

bool bc_radio_pub_push_button(uint16_t *event_count);

bool bc_radio_pub_thermometer(float *temperature);

#endif // _BC_RADIO_H
