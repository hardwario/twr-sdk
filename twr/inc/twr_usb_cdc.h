#ifndef _TWR_USB_CDC_H
#define _TWR_USB_CDC_H

#include <twr_common.h>

//! @addtogroup twr_usb_cdc twr_usb_cdc
//! @brief USB CDC communication library
//! @{

//! @brief Initialize USB CDC library

void twr_usb_cdc_init(void);

//! @brief Write buffer to USB CDC (non-blocking call)
//! @param[in] buffer Pointer to buffer to be written
//! @param[in] length Number of bytes to be written
//! @return true On success
//! @return false On failure

bool twr_usb_cdc_write(const void *buffer, size_t length);

//! @brief Read buffer from USB CDC (non-blocking call)
//! @param[out] buffer Pointer to buffer to be read
//! @param[in] length Number of bytes to be read
//! @return Number of bytes read

size_t twr_usb_cdc_read(void *buffer, size_t length);

//! @}

#endif // _TWR_USB_CDC_H
