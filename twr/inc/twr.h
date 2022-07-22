#ifndef _TWR_H
#define _TWR_H

#ifndef FW_VERSION
#define FW_VERSION "vdev"
#endif

#ifndef GIT_VERSION
#define GIT_VERSION "?"
#endif

#ifndef BUILD_DATE
#define BUILD_DATE ""
#endif

// Miscellaneous

#include <twr_info.h>
#include <twr_ir_rx.h>
#include <twr_irq.h>
#include <twr_led_strip.h>
#include <twr_log.h>
#include <twr_radio_node.h>
#include <twr_radio_pub.h>
#include <twr_radio.h>

// Peripheral drivers

#include <twr_adc.h>
#include <twr_button.h>
#include <twr_dac.h>
#include <twr_eeprom.h>
#include <twr_gpio.h>
#include <twr_i2c.h>
#include <twr_led.h>
#include <twr_pwm.h>
#include <twr_rtc.h>
#include <twr_spi.h>
#include <twr_uart.h>

// Chip drivers

#include <twr_cmwx1zzabz.h>
#include <twr_cp201t.h>
#include <twr_ds2484.h>
#include <twr_esp8266.h>
#include <twr_hc_sr04.h>
#include <twr_lis2dh12.h>
#include <twr_lp8.h>
#include <twr_ls013b7dh03.h>
#include <twr_sam_m8q.h>
#include <twr_sgp30.h>
#include <twr_sgpc3.h>
#include <twr_spirit1.h>

// HARDWARIO tags

#include <twr_tag_barometer.h>
#include <twr_tag_humidity.h>
#include <twr_tag_lux_meter.h>
#include <twr_tag_nfc.h>
#include <twr_tag_temperature.h>
#include <twr_tag_voc_lp.h>
#include <twr_tag_voc.h>

// HARDWARIO modules

#include <twr_module_battery.h>
#include <twr_module_climate.h>
#include <twr_module_co2.h>
#include <twr_module_encoder.h>
#include <twr_module_gps.h>
#include <twr_module_infra_grid.h>
#include <twr_module_lcd.h>
#include <twr_module_pir.h>
#include <twr_module_power.h>
#include <twr_module_relay.h>
#include <twr_module_rs485.h>
#include <twr_module_sensor.h>
#include <twr_module_sigfox.h>
#include <twr_module_x1.h>

// Other

#include <twr_at_lora.h>
#include <twr_analog_sensor.h>
#include <twr_atci.h>
#include <twr_base64.h>
#include <twr_chester_a.h>
#include <twr_config.h>
#include <twr_data_stream.h>
#include <twr_delay.h>
#include <twr_dice.h>
#include <twr_ds18b20.h>
#include <twr_error.h>
#include <twr_flood_detector.h>
#include <twr_font_common.h>
#include <twr_gfx.h>
#include <twr_image.h>
#include <twr_onewire_ds2484.h>
#include <twr_onewire_gpio.h>
#include <twr_onewire_relay.h>
#include <twr_onewire.h>
#include <twr_pulse_counter.h>
#include <twr_queue.h>
#include <twr_ramp.h>
#include <twr_sha256.h>
#include <twr_soil_sensor.h>
#include <twr_switch.h>
#include <twr_system.h>
#include <twr_timer.h>
#include <twr_usb_cdc.h>

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

#endif // _TWR_H
