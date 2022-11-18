#include <application.h>

twr_led_t led;

twr_lis2dh12_t a;
twr_lis2dh12_result_g_t a_result;

void lis2_event_handler(twr_lis2dh12_t *self, twr_lis2dh12_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == TWR_LIS2DH12_EVENT_UPDATE) {
        twr_lis2dh12_get_result_g(&a, &a_result);
        twr_log_debug("X: %f\r\nY: %f\r\nZ: %f\r\n", a_result.x_axis, a_result.y_axis, a_result.z_axis);
    } else {
        twr_log_debug("error");
    }
}

void application_init(void)
{
    twr_log_init(TWR_LOG_LEVEL_DEBUG, TWR_LOG_TIMESTAMP_OFF);

    twr_lis2dh12_init(&a, TWR_I2C_I2C0, 0x19);
    twr_lis2dh12_set_event_handler(&a, lis2_event_handler, NULL);
    twr_lis2dh12_set_update_interval(&a, 1000);
}
