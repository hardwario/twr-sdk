#ifndef _BCL_H
#define _BCL_H

// CORE MODULE STUFF
#include <usb_talk.h>
#include <bc_led.h>
#include <bc_button.h>
#include <bc_lis2dh12.h>
#include <bc_spirit1.h>
#include <bc_gpio.h>
#include <bc_i2c.h>
#include <bc_irq.h>

// TAGS
#include <bc_tag_barometer.h>
#include <bc_tag_humidity.h>
#include <bc_tag_lux_meter.h>
#include <bc_tag_temperature.h>

// MODULES
#include <bc_module_pir.h>
#include <bc_module_power.h>
#include <bc_module_relay.h>
#include <bc_module_sigfox.h>

#endif // _BCL_H
