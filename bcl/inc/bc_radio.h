#ifndef _BC_RADIO_H
#define _BC_RADIO_H

#include <bc_common.h>
#include <bc_button.h>
#include <bc_led.h>

#ifndef BC_RADIO_MAX_DEVICES
#define BC_RADIO_MAX_DEVICES 8
#endif

typedef enum
{
    BC_RADIO_EVENT_INIT_FAILURE = 0,
    BC_RADIO_EVENT_INIT_DONE = 1,
    BC_RADIO_EVENT_ATTACH = 2,
    BC_RADIO_EVENT_ATTACH_FAILURE = 3,
    BC_RADIO_EVENT_DETACH = 4,
	BC_RADIO_EVENT_SCAN_FIND_DEVICE = 5,

} bc_radio_event_t;

void bc_radio_init(void);

void bc_radio_set_event_handler(void (*event_handler)(bc_radio_event_t, void *), void *event_param);

void bc_radio_listen(void);

void bc_radio_sleep(void);

void bc_radio_enroll_to_gateway(void);

void bc_radio_enrollment_start(void);

void bc_radio_enrollment_stop(void);

bool bc_radio_peer_device_add(uint64_t device_address);

bool bc_radio_peer_device_remove(uint64_t device_address);

bool bc_radio_peer_device_purge_all(void);

void bc_radio_get_peer_devices_address(uint64_t *device_address, int length);

void bc_radio_scan_start(void);

void bc_radio_scan_stop(void);

void bc_radio_automatic_pairing_start(void);

void bc_radio_automatic_pairing_stop(void);

uint64_t bc_radio_get_device_address(void);

uint64_t bc_radio_get_event_device_address(void);

bool bc_radio_pub_push_button(uint16_t *event_count);

bool bc_radio_pub_thermometer(uint8_t i2c, float *temperature);

bool bc_radio_pub_humidity(uint8_t i2c, float *percentage);

bool bc_radio_pub_luminosity(uint8_t i2c, float *lux);

bool bc_radio_pub_barometer(uint8_t i2c, float *pascal, float *meter);

bool bc_radio_pub_co2(float *concentration);

bool bc_radio_pub_battery(uint8_t format, float *voltage);

bool bc_radio_pub_buffer(void *buffer, size_t length);

bool bc_radio_pub_info(char *firmware);

void bc_radio_init_pairing_button();

#endif // _BC_RADIO_H
