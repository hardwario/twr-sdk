#ifndef _BCL_H
#define _BCL_H

// Miscellaneous

#include <bc_irq.h>
#include <bc_radio.h>
#include <usb_talk.h>
#include <bc_led_strip.h>

// Peripheral drivers

#include <bc_button.h>
#include <bc_dac.h>
#include <bc_eeprom.h>
#include <bc_gpio.h>
#include <bc_i2c.h>
#include <bc_led.h>
#include <bc_rtc.h>
#include <bc_uart.h>

// Chip drivers

#include <bc_lis2dh12.h>
#include <bc_spirit1.h>
#include <bc_hc_sr04.h>

// BigClown tags

#include <bc_tag_barometer.h>
#include <bc_tag_humidity.h>
#include <bc_tag_lux_meter.h>
#include <bc_tag_temperature.h>

// BigClown modules

#include <bc_module_climate.h>
#include <bc_module_encoder.h>
#include <bc_module_pir.h>
#include <bc_module_power.h>
#include <bc_module_relay.h>
#include <bc_module_sigfox.h>
#include <bc_module_lcd.h>

//! @mainpage BigClown firmware SDK
//! This is API documentation of BigClown SDK

#endif // _BCL_H
