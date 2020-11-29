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

hio_led_t led;

static void spirit1_event_handler(hio_spirit1_event_t event, void *event_param);

void temperature_tag_event_handler(hio_tag_temperature_t *self, hio_tag_temperature_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    i2c_sensors.temperature.valid = hio_tag_temperature_get_temperature_celsius(self, &i2c_sensors.temperature.value);
}

void humidity_tag_event_handler(hio_tag_humidity_t *self, hio_tag_humidity_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    i2c_sensors.humidity.valid = hio_tag_humidity_get_humidity_percentage(self, &i2c_sensors.humidity.value);
}

void humidity_tag_event_handler_r1(hio_tag_humidity_t *self, hio_tag_humidity_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    i2c_sensors.humidity_r1.valid = hio_tag_humidity_get_humidity_percentage(self, &i2c_sensors.humidity_r1.value);
}

void lux_meter_event_handler(hio_tag_lux_meter_t *self, hio_tag_lux_meter_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    i2c_sensors.luminosity.valid = hio_tag_lux_meter_get_illuminance_lux(self, &i2c_sensors.luminosity.value);
}

void barometer_tag_event_handler(hio_tag_barometer_t *self, hio_tag_barometer_event_t event, void *event_param)
{
    (void) event;
    (void) event_param;

    i2c_sensors.altitude.valid = hio_tag_barometer_get_altitude_meter(self, &i2c_sensors.altitude.value);
    i2c_sensors.pressure.valid = hio_tag_barometer_get_pressure_pascal(self, &i2c_sensors.pressure.value);
}

void button_event_handler(hio_button_t *self, hio_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == HIO_BUTTON_EVENT_PRESS)
    {
        hio_led_pulse(&led, 100);

        hio_spirit1_set_tx_length(16);
        hio_spirit1_tx();
    }

    if (event == HIO_BUTTON_EVENT_RELEASE)
    {
        hio_led_pulse(&led, 100);
    }
}

void lis2dh12_event_handler(hio_lis2dh12_t *self, hio_lis2dh12_event_t event, void *event_param)
{
    (void) event_param;

    if (event == HIO_LIS2DH12_EVENT_UPDATE)
    {
        hio_lis2dh12_result_g_t result;

        if (hio_lis2dh12_get_result_g(self, &result))
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
    else if (event == HIO_LIS2DH12_EVENT_ALARM)
    {
        hio_led_pulse(&led, 100);
    }
}

hio_tag_humidity_t humidity_tag;
hio_tag_humidity_t humidity_tag_r1;

void application_init(void)
{
    hio_led_init(&led, HIO_GPIO_LED, false, false);
    //hio_led_set_mode(&led, HIO_LED_MODE_BLINK);

    static hio_button_t button;

    hio_button_init(&button, HIO_GPIO_BUTTON, HIO_GPIO_PULL_DOWN, false);
    hio_button_set_event_handler(&button, button_event_handler, NULL);

    hio_i2c_init(HIO_I2C_I2C0, HIO_I2C_SPEED_400_KHZ);

    static hio_tag_temperature_t temperature_tag;

    hio_tag_temperature_init(&temperature_tag, HIO_I2C_I2C0, HIO_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE);
    hio_tag_temperature_set_update_interval(&temperature_tag, 1000);
    hio_tag_temperature_set_event_handler(&temperature_tag, temperature_tag_event_handler, NULL);

    hio_tag_humidity_init(&humidity_tag, HIO_TAG_HUMIDITY_REVISION_R2, HIO_I2C_I2C0, HIO_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT);
    hio_tag_humidity_set_update_interval(&humidity_tag, 1000);
    hio_tag_humidity_set_event_handler(&humidity_tag, humidity_tag_event_handler, NULL);

    hio_tag_humidity_init(&humidity_tag_r1, HIO_TAG_HUMIDITY_REVISION_R1, HIO_I2C_I2C0, HIO_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT);
    hio_tag_humidity_set_update_interval(&humidity_tag_r1, 1000);
    hio_tag_humidity_set_event_handler(&humidity_tag_r1, humidity_tag_event_handler_r1, NULL);

    static hio_tag_lux_meter_t lux_meter;

    hio_tag_lux_meter_init(&lux_meter, HIO_I2C_I2C0, HIO_TAG_LUX_METER_I2C_ADDRESS_DEFAULT);
    hio_tag_lux_meter_set_update_interval(&lux_meter, 1000);
    hio_tag_lux_meter_set_event_handler(&lux_meter, lux_meter_event_handler, NULL);

    static hio_tag_barometer_t barometer_tag;

    hio_tag_barometer_init(&barometer_tag, HIO_I2C_I2C0);
    hio_tag_barometer_set_update_interval(&barometer_tag, 1000);
    hio_tag_barometer_set_event_handler(&barometer_tag, barometer_tag_event_handler, NULL);

    static hio_lis2dh12_t lis2dh12;

    hio_lis2dh12_alarm_t alarm;

    alarm.x_high = false;
    alarm.x_low = false;
    alarm.y_high = false;
    alarm.y_low = false;
    alarm.z_high = true;
    alarm.z_low = false;
    alarm.threshold = .5f;
    alarm.duration = 1;

    hio_lis2dh12_init(&lis2dh12, HIO_I2C_I2C0, 0x19);
    hio_lis2dh12_set_update_interval(&lis2dh12, 100);
    hio_lis2dh12_set_alarm(&lis2dh12, &alarm);
    hio_lis2dh12_set_event_handler(&lis2dh12, lis2dh12_event_handler, NULL);

    hio_spirit1_init();
    hio_spirit1_set_event_handler(spirit1_event_handler, NULL);
    hio_spirit1_rx();
}

static void spirit1_event_handler(hio_spirit1_event_t event, void *event_param)
{
    (void) event_param;

    if (event == HIO_SPIRIT1_EVENT_RX_DONE)
    {
        hio_led_pulse(&led, 100);
    }

    if (event == HIO_SPIRIT1_EVENT_TX_DONE)
    {
        hio_spirit1_rx();
    }
}
