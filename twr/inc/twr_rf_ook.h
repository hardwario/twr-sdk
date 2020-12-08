#ifndef _TWR_RF_OOK_H
#define _TWR_RF_OOK_H

#include <twr_system.h>
#include <twr_gpio.h>
#include <stm32l0xx.h>

//! @addtogroup twr_rf_ook twr_rf_ook
//! @brief Driver for ON-OFF-KEY modulation for 433 MHz Radio modules
//! @{

//! @brief Initialize RF OOK library
//! @param[in] gpio GPIO pin

void twr_rf_ook_init(twr_gpio_channel_t gpio);

//! @brief Configure OOK bitrate
//! @param[in] bitrate in bits per second

void twr_rf_ook_set_bitrate(uint32_t bitrate);

//! @brief Configure OOK bitrate
//! @param[in] bit_length_us length of a single bit in microseconds

void twr_rf_ook_set_bitlength(uint32_t bit_length_us);

//! @brief Send data
//! @param[in] packet packet
//! @param[in] len packet length in bytes

bool twr_rf_ook_send(uint8_t *packet, uint8_t length);

//! @brief Send data with data in hex string
//! @param[in] hex_string hex string with data to send

bool twr_rf_ook_send_hex_string(char *hex_string);

//! @brief Data sending in progress

bool twr_rf_ook_is_busy();

//! @brief Data can be send

bool twr_rf_ook_is_ready();

//! @}

#endif // _TWR_RF_OOK_H
