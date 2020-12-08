#include <application.h>

int found_devices;
uint64_t device_list[5];
twr_onewire_t *onewire;

void application_init(void)
{

    twr_module_sensor_init();

    twr_module_sensor_onewire_power_up();

    onewire = twr_module_sensor_get_onewire();
}

void application_task()
{
    // Search on channel B
    memset(device_list, 0x00, sizeof(device_list));
    found_devices = twr_onewire_search_all(onewire, device_list, sizeof(device_list));

    twr_scheduler_plan_current_relative(1000);
}
