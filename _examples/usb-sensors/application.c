/*

Visual Studio Code
Ctrl+Shift+B     to build
Ctrl+P task dfu  to flash MCU with dfu-util

*/

#include <application.h>

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

twr_led_t led;

static void spirit1_event_handler(twr_spirit1_event_t event, void *event_param);

void temperature_tag_event_handler(twr_tag_temperature_t *self, twr_tag_temperature_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    i2c_sensors.temperature.valid = twr_tag_temperature_get_temperature_celsius(self, &i2c_sensors.temperature.value);
}

void humidity_tag_event_handler(twr_tag_humidity_t *self, twr_tag_humidity_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    i2c_sensors.humidity.valid = twr_tag_humidity_get_humidity_percentage(self, &i2c_sensors.humidity.value);
}

void humidity_tag_event_handler_r1(twr_tag_humidity_t *self, twr_tag_humidity_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    i2c_sensors.humidity_r1.valid = twr_tag_humidity_get_humidity_percentage(self, &i2c_sensors.humidity_r1.value);
}

void lux_meter_event_handler(twr_tag_lux_meter_t *self, twr_tag_lux_meter_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    i2c_sensors.luminosity.valid = twr_tag_lux_meter_get_illuminance_lux(self, &i2c_sensors.luminosity.value);
}

void barometer_tag_event_handler(twr_tag_barometer_t *self, twr_tag_barometer_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    i2c_sensors.altitude.valid = twr_tag_barometer_get_altitude_meter(self, &i2c_sensors.altitude.value);
    i2c_sensors.pressure.valid = twr_tag_barometer_get_pressure_pascal(self, &i2c_sensors.pressure.value);
}

void button_event_handler(twr_button_t *self, twr_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == TWR_BUTTON_EVENT_PRESS)
    {
        twr_led_pulse(&led, 100);

        twr_spirit1_set_tx_length(16);
        twr_spirit1_tx();
    }

    if (event == TWR_BUTTON_EVENT_RELEASE)
    {
        twr_led_pulse(&led, 100);
    }
}

void lis2dh12_event_handler(twr_lis2dh12_t *self, twr_lis2dh12_event_t event, void *event_param)
{
    (void) event_param;

    if (event == TWR_LIS2DH12_EVENT_UPDATE)
    {
        twr_lis2dh12_result_g_t result;

        if (twr_lis2dh12_get_result_g(self, &result))
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
    else if (event == TWR_LIS2DH12_EVENT_ALARM)
    {
        twr_led_pulse(&led, 100);
    }
}

twr_tag_humidity_t humidity_tag;
twr_tag_humidity_t humidity_tag_r1;

void application_init(void)
{
    twr_led_init(&led, TWR_GPIO_LED, false, false);
    //twr_led_set_mode(&led, TWR_LED_MODE_BLINK);

    static twr_button_t button;

    twr_button_init(&button, TWR_GPIO_BUTTON, TWR_GPIO_PULL_DOWN, false);
    twr_button_set_event_handler(&button, button_event_handler, NULL);

    twr_i2c_init(TWR_I2C_I2C0, TWR_I2C_SPEED_400_KHZ);

    static twr_tag_temperature_t temperature_tag;

    twr_tag_temperature_init(&temperature_tag, TWR_I2C_I2C0, TWR_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE);
    twr_tag_temperature_set_update_interval(&temperature_tag, 1000);
    twr_tag_temperature_set_event_handler(&temperature_tag, temperature_tag_event_handler, NULL);

    twr_tag_humidity_init(&humidity_tag, TWR_TAG_HUMIDITY_REVISION_R2, TWR_I2C_I2C0, TWR_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT);
    twr_tag_humidity_set_update_interval(&humidity_tag, 1000);
    twr_tag_humidity_set_event_handler(&humidity_tag, humidity_tag_event_handler, NULL);

    twr_tag_humidity_init(&humidity_tag_r1, TWR_TAG_HUMIDITY_REVISION_R1, TWR_I2C_I2C0, TWR_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT);
    twr_tag_humidity_set_update_interval(&humidity_tag_r1, 1000);
    twr_tag_humidity_set_event_handler(&humidity_tag_r1, humidity_tag_event_handler_r1, NULL);

    static twr_tag_lux_meter_t lux_meter;

    twr_tag_lux_meter_init(&lux_meter, TWR_I2C_I2C0, TWR_TAG_LUX_METER_I2C_ADDRESS_DEFAULT);
    twr_tag_lux_meter_set_update_interval(&lux_meter, 1000);
    twr_tag_lux_meter_set_event_handler(&lux_meter, lux_meter_event_handler, NULL);

    static twr_tag_barometer_t barometer_tag;

    twr_tag_barometer_init(&barometer_tag, TWR_I2C_I2C0);
    twr_tag_barometer_set_update_interval(&barometer_tag, 1000);
    twr_tag_barometer_set_event_handler(&barometer_tag, barometer_tag_event_handler, NULL);

    static twr_lis2dh12_t lis2dh12;

    twr_lis2dh12_alarm_t alarm;

    alarm.x_high = false;
    alarm.x_low = false;
    alarm.y_high = false;
    alarm.y_low = false;
    alarm.z_high = true;
    alarm.z_low = false;
    alarm.threshold = .5f;
    alarm.duration = 1;

    twr_lis2dh12_init(&lis2dh12, TWR_I2C_I2C0, 0x19);
    twr_lis2dh12_set_update_interval(&lis2dh12, 100);
    twr_lis2dh12_set_alarm(&lis2dh12, &alarm);
    twr_lis2dh12_set_event_handler(&lis2dh12, lis2dh12_event_handler, NULL);

    twr_spirit1_init();
    twr_spirit1_set_event_handler(spirit1_event_handler, NULL);
    twr_spirit1_rx();
}

static void spirit1_event_handler(twr_spirit1_event_t event, void *event_param)
{
    (void) event_param;

    if (event == TWR_SPIRIT1_EVENT_RX_DONE)
    {
        twr_led_pulse(&led, 100);
    }

    if (event == TWR_SPIRIT1_EVENT_TX_DONE)
    {
        twr_spirit1_rx();
    }
}
