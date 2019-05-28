#ifndef _BCL_H
#define _BCL_H

// Miscellaneous

#include <bc_ir_rx.h>
#include <bc_irq.h>
#include <bc_led_strip.h>
#include <bc_radio.h>
#include <bc_radio_pub.h>
#include <bc_radio_node.h>
#include <bc_log.h>

// Peripheral drivers

#include <bc_adc.h>
#include <bc_button.h>
#include <bc_dac.h>
#include <bc_eeprom.h>
#include <bc_gpio.h>
#include <bc_i2c.h>
#include <bc_led.h>
#include <bc_rtc.h>
#include <bc_uart.h>
#include <bc_spi.h>
#include <bc_pwm.h>

// Chip drivers

#include <bc_sgp30.h>
#include <bc_sgpc3.h>
#include <bc_cp201t.h>
#include <bc_hc_sr04.h>
#include <bc_lis2dh12.h>
#include <bc_lp8.h>
#include <bc_spirit1.h>
#include <bc_ls013b7dh03.h>
#include <bc_cmwx1zzabz.h>
#include <bc_sam_m8q.h>

// BigClown tags

#include <bc_tag_barometer.h>
#include <bc_tag_humidity.h>
#include <bc_tag_lux_meter.h>
#include <bc_tag_temperature.h>
#include <bc_tag_nfc.h>
#include <bc_tag_voc.h>
#include <bc_tag_voc_lp.h>

// BigClown modules

#include <bc_module_battery.h>
#include <bc_module_climate.h>
#include <bc_module_encoder.h>
#include <bc_module_gps.h>
#include <bc_module_pir.h>
#include <bc_module_power.h>
#include <bc_module_relay.h>
#include <bc_module_rs485.h>
#include <bc_module_sigfox.h>
#include <bc_module_lcd.h>
#include <bc_module_co2.h>
#include <bc_module_sensor.h>
#include <bc_module_infra_grid.h>

// Other

#include <bc_analog_sensor.h>
#include <bc_data_stream.h>
#include <bc_flood_detector.h>
#include <bc_pulse_counter.h>
#include <bc_soil_sensor.h>
#include <bc_font_common.h>
#include <bc_image.h>
#include <bc_system.h>
#include <bc_switch.h>
#include <bc_timer.h>
#include <bc_error.h>
#include <bc_dice.h>
#include <bc_ramp.h>
#include <bc_gfx.h>
#include <bc_atci.h>
#include <bc_base64.h>
#include <bc_sha256.h>

#pragma GCC diagnostic ignored "-Wunused-parameter"

//! @mainpage Overview
//! Here you will find all the documented firmware SDK APIs.
//!
//! The SDK currently supports the following hardware platforms:
//! \li BigClown Core Module
//! \li BigClown USB Dongle
//! \li BigClown Cloony
//!
//! Go to the <a href="modules.html"><b>Modules</b></a> page for complete listing.
//!
//! For more information visit the <a href="https://www.bigclown.com/doc/firmware/basic-overview/" target="_blank"><b>Basic Overview</b></a> document in BigClown Documentation.

#endif // _BCL_H
