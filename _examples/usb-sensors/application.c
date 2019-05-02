/*

Visual Studio Code
Ctrl+Shift+B     to build
Ctrl+P task dfu  to flash MCU with dfu-util

*/

#include <application.h>
#include <bc_led.h>
#include <bc_button.h>
#include <bc_i2c.h>
#include <bc_tag_temperature.h>
#include <bc_tag_humidity.h>
#include <bc_tag_lux_meter.h>
#include <bc_tag_barometer.h>
#include <bc_lis2dh12.h>

#include <bc_spirit1.h>

struct
{
    struct { bool valid; float value; } temperature;
    struct { bool valid; float value; } humidity;
    struct { bool valid; float value; } humidity_r1;
    struct { bool valid; float value; } luminosity;
    struct { bool valid; float value; } altitude;
    struct { bool valid; float value; } pressure;
    struct { bool valid; float value; } acceleration_x;
    struct { bool valid; float value; } acceleration_y;
    struct { bool valid; float value; } acceleration_z;

} i2c_sensors;

bc_led_t led;

static void spirit1_event_handler(bc_spirit1_event_t event, void *event_param);

void temperature_tag_event_handler(bc_tag_temperature_t *self, bc_tag_temperature_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    i2c_sensors.temperature.valid = bc_tag_temperature_get_temperature_celsius(self, &i2c_sensors.temperature.value);
}

void humidity_tag_event_handler(bc_tag_humidity_t *self, bc_tag_humidity_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    i2c_sensors.humidity.valid = bc_tag_humidity_get_humidity_percentage(self, &i2c_sensors.humidity.value);
}

void humidity_tag_event_handler_r1(bc_tag_humidity_t *self, bc_tag_humidity_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    i2c_sensors.humidity_r1.valid = bc_tag_humidity_get_humidity_percentage(self, &i2c_sensors.humidity_r1.value);
}

void lux_meter_event_handler(bc_tag_lux_meter_t *self, bc_tag_lux_meter_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    i2c_sensors.luminosity.valid = bc_tag_lux_meter_get_illuminance_lux(self, &i2c_sensors.luminosity.value);
}

void barometer_tag_event_handler(bc_tag_barometer_t *self, bc_tag_barometer_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    i2c_sensors.altitude.valid = bc_tag_barometer_get_altitude_meter(self, &i2c_sensors.altitude.value);
    i2c_sensors.pressure.valid = bc_tag_barometer_get_pressure_pascal(self, &i2c_sensors.pressure.value);
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
        bc_led_pulse(&led, 100);

        bc_spirit1_set_tx_length(16);
        bc_spirit1_tx();
    }

    if (event == BC_BUTTON_EVENT_RELEASE)
    {
        bc_led_pulse(&led, 100);
    }
}

void lis2dh12_event_handler(bc_lis2dh12_t *self, bc_lis2dh12_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_LIS2DH12_EVENT_UPDATE)
    {
        bc_lis2dh12_result_g_t result;

        if (bc_lis2dh12_get_result_g(self, &result))
        {
            i2c_sensors.acceleration_x.value = result.x_axis;
            i2c_sensors.acceleration_y.value = result.y_axis;
            i2c_sensors.acceleration_z.value = result.z_axis;

            i2c_sensors.acceleration_x.valid = true;
            i2c_sensors.acceleration_y.valid = true;
            i2c_sensors.acceleration_z.valid = true;
        }
        else
        {
            i2c_sensors.acceleration_x.valid = false;
            i2c_sensors.acceleration_y.valid = false;
            i2c_sensors.acceleration_z.valid = false;
        }
    }
    else if (event == BC_LIS2DH12_EVENT_ALARM)
    {
        bc_led_pulse(&led, 100);
    }
}

bc_tag_humidity_t humidity_tag;
bc_tag_humidity_t humidity_tag_r1;

void application_init(void)
{
    bc_led_init(&led, BC_GPIO_LED, false, false);
    //bc_led_set_mode(&led, BC_LED_MODE_BLINK);

    static bc_button_t button;

    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    bc_i2c_init(BC_I2C_I2C0, BC_I2C_SPEED_400_KHZ);

    static bc_tag_temperature_t temperature_tag;

    bc_tag_temperature_init(&temperature_tag, BC_I2C_I2C0, BC_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE);
    bc_tag_temperature_set_update_interval(&temperature_tag, 1000);
    bc_tag_temperature_set_event_handler(&temperature_tag, temperature_tag_event_handler, NULL);

    bc_tag_humidity_init(&humidity_tag, BC_TAG_HUMIDITY_REVISION_R2, BC_I2C_I2C0, BC_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT);
    bc_tag_humidity_set_update_interval(&humidity_tag, 1000);
    bc_tag_humidity_set_event_handler(&humidity_tag, humidity_tag_event_handler, NULL);

    bc_tag_humidity_init(&humidity_tag_r1, BC_TAG_HUMIDITY_REVISION_R1, BC_I2C_I2C0, BC_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT);
    bc_tag_humidity_set_update_interval(&humidity_tag_r1, 1000);
    bc_tag_humidity_set_event_handler(&humidity_tag_r1, humidity_tag_event_handler_r1, NULL);

    static bc_tag_lux_meter_t lux_meter;

    bc_tag_lux_meter_init(&lux_meter, BC_I2C_I2C0, BC_TAG_LUX_METER_I2C_ADDRESS_DEFAULT);
    bc_tag_lux_meter_set_update_interval(&lux_meter, 1000);
    bc_tag_lux_meter_set_event_handler(&lux_meter, lux_meter_event_handler, NULL);

    static bc_tag_barometer_t barometer_tag;

    bc_tag_barometer_init(&barometer_tag, BC_I2C_I2C0);
    bc_tag_barometer_set_update_interval(&barometer_tag, 1000);
    bc_tag_barometer_set_event_handler(&barometer_tag, barometer_tag_event_handler, NULL);

    static bc_lis2dh12_t lis2dh12;

    bc_lis2dh12_alarm_t alarm;

    alarm.x_high = false;
    alarm.x_low = false;
    alarm.y_high = false;
    alarm.y_low = false;
    alarm.z_high = true;
    alarm.z_low = false;
    alarm.threshold = .5f;
    alarm.duration = 1;

    bc_lis2dh12_init(&lis2dh12, BC_I2C_I2C0, 0x19);
    bc_lis2dh12_set_update_interval(&lis2dh12, 100);
    bc_lis2dh12_set_alarm(&lis2dh12, &alarm);
    bc_lis2dh12_set_event_handler(&lis2dh12, lis2dh12_event_handler, NULL);

    bc_spirit1_init();
    bc_spirit1_set_event_handler(spirit1_event_handler, NULL);
    bc_spirit1_rx();
}

static void spirit1_event_handler(bc_spirit1_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_SPIRIT1_EVENT_RX_DONE)
    {
        bc_led_pulse(&led, 100);
    }

    if (event == BC_SPIRIT1_EVENT_TX_DONE)
    {
        bc_spirit1_rx();
    }
}
