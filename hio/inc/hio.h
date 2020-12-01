#ifndef _HIO_H
#define _HIO_H

// Miscellaneous

#include <hio_ir_rx.h>
#include <hio_irq.h>
#include <hio_led_strip.h>
#include <hio_radio.h>
#include <hio_radio_pub.h>
#include <hio_radio_node.h>
#include <hio_log.h>

// Peripheral drivers

#include <hio_adc.h>
#include <hio_button.h>
#include <hio_dac.h>
#include <hio_eeprom.h>
#include <hio_gpio.h>
#include <hio_i2c.h>
#include <hio_led.h>
#include <hio_rtc.h>
#include <hio_uart.h>
#include <hio_spi.h>
#include <hio_pwm.h>

// Chip drivers

#include <hio_sgp30.h>
#include <hio_sgpc3.h>
#include <hio_cp201t.h>
#include <hio_hc_sr04.h>
#include <hio_lis2dh12.h>
#include <hio_lp8.h>
#include <hio_spirit1.h>
#include <hio_ls013b7dh03.h>
#include <hio_cmwx1zzabz.h>
#include <hio_sam_m8q.h>
#include <hio_esp8266.h>
#include <hio_ds2484.h>

// HARDWARIO tags

#include <hio_tag_barometer.h>
#include <hio_tag_humidity.h>
#include <hio_tag_lux_meter.h>
#include <hio_tag_temperature.h>
#include <hio_tag_nfc.h>
#include <hio_tag_voc.h>
#include <hio_tag_voc_lp.h>

// HARDWARIO modules

#include <hio_module_battery.h>
#include <hio_module_climate.h>
#include <hio_module_encoder.h>
#include <hio_module_gps.h>
#include <hio_module_pir.h>
#include <hio_module_power.h>
#include <hio_module_relay.h>
#include <hio_module_rs485.h>
#include <hio_module_sigfox.h>
#include <hio_module_lcd.h>
#include <hio_module_co2.h>
#include <hio_module_sensor.h>
#include <hio_module_infra_grid.h>
#include <hio_module_x1.h>

// Other

#include <hio_analog_sensor.h>
#include <hio_config.h>
#include <hio_data_stream.h>
#include <hio_flood_detector.h>
#include <hio_pulse_counter.h>
#include <hio_soil_sensor.h>
#include <hio_font_common.h>
#include <hio_image.h>
#include <hio_system.h>
#include <hio_switch.h>
#include <hio_timer.h>
#include <hio_error.h>
#include <hio_dice.h>
#include <hio_ramp.h>
#include <hio_gfx.h>
#include <hio_atci.h>
#include <hio_base64.h>
#include <hio_sha256.h>
#include <hio_ds18b20.h>
#include <hio_onewire.h>
#include <hio_onewire_gpio.h>
#include <hio_onewire_ds2484.h>
#include <hio_onewire_relay.h>
#include <hio_delay.h>
#include <hio_usb_cdc.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"

//! @mainpage Overview
//! Here you will find all the documented firmware SDK APIs.
//!
//! The SDK currently supports the following hardware platforms:
//! \li HARDWARIO Core Module
//! \li HARDWARIO USB Dongle
//! \li HARDWARIO Cloony
//!
//! Go to the <a href="modules.html"><b>Modules</b></a> page for complete listing.
//!
//! For more information visit the <a href="https://www.bigclown.com/doc/firmware/basic-overview/" target="_blank"><b>Basic Overview</b></a> document in BigClown Documentation.

#endif // _HIO_H
