#include <application.h>

int found_devices;
uint64_t device_list[5];
hio_onewire_t *onewire;

void application_init(void)
{

    hio_module_sensor_init();

    hio_module_sensor_onewire_power_up();

    onewire = hio_module_sensor_get_onewire();
}

void application_task()
{
    // Search on channel B
    memset(device_list, 0x00, sizeof(device_list));
    found_devices = hio_onewire_search_all(onewire, device_list, sizeof(device_list));

    hio_scheduler_plan_current_relative(1000);
}
