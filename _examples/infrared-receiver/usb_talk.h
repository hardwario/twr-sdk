#ifndef _USB_TALK_H
#define _USB_TALK_H

#include <bc_common.h>

typedef struct
{
    char tx_buffer[1024];
    char rx_buffer[1024];
    size_t rx_length;
    bool rx_error;

    uint8_t pixels[150 * 4];
    bool light_is_on;

} usb_talk_t;

extern usb_talk_t usb_talk;

void usb_talk_init(void);
void usb_talk_publish_push_button(const char *prefix, uint16_t *event_count);
void usb_talk_publish_light(void);
void usb_talk_publish_relay(void);
void usb_talk_publish_led_strip_config(const char *sufix);
void usb_talk_publish_thermometer(const char *prefix, float *temperature);
void usb_talk_publish_humidity_sensor(const char *prefix, float *relative_humidity);
void usb_talk_publish_ir_rx(const char *prefix, uint32_t *ir_code);
void usb_talk_received_data(uint8_t *buffer, size_t length);
void usb_talk_publish_input_change(const char *prefix, uint16_t *event_count);

#endif /* _USB_TALK_H */
