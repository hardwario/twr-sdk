#ifndef _BC_USB_CDC_H
#define _BC_USB_CDC_H

#include <bc_common.h>

void bc_usb_cdc_init(void);
void bc_usb_cdc_start(void);
bool bc_usb_cdc_write(const void *buffer, size_t length);
size_t bc_usb_cdc_read(void *buffer, size_t length);

#endif /* _BC_USB_CDC_H */
