#ifndef _HIO_RF_OOK_H
#define _HIO_RF_OOK_H

#include <hio_system.h>
#include <hio_gpio.h>
#include <stm32l0xx.h>

//! @addtogroup hio_rf_ook hio_rf_ook
//! @brief Driver for ON-OFF-KEY modulation for 433 MHz Radio modules
//! @{

//! @brief Initialize RF OOK library
//! @param[in] gpio GPIO pin

void hio_rf_ook_init(hio_gpio_channel_t gpio);

//! @brief Configure OOK bitrate
//! @param[in] bitrate in bits per second

void hio_rf_ook_set_bitrate(uint32_t bitrate);

//! @brief Configure OOK bitrate
//! @param[in] bit_length_us length of a single bit in microseconds

void hio_rf_ook_set_bitlength(uint32_t bit_length_us);

//! @brief Send data
//! @param[in] packet packet
//! @param[in] len packet length in bytes

bool hio_rf_ook_send(uint8_t *packet, uint8_t length);

//! @brief Send data with data in hex string
//! @param[in] hex_string hex string with data to send

bool hio_rf_ook_send_hex_string(char *hex_string);

//! @brief Data sending in progress

bool hio_rf_ook_is_busy();

//! @brief Data can be send

bool hio_rf_ook_is_ready();

//! @}

#endif // _HIO_RF_OOK_H
