#include <application.h>

int found_devices;
uint64_t device_list[5];
bc_onewire_t *onewire;

void application_init(void)
{

    bc_module_sensor_init();

    bc_module_sensor_onewire_power_up();

    onewire = bc_module_sensor_get_onewire();
}

void application_task()
{
    // Search on channel B
    memset(device_list, 0x00, sizeof(device_list));
    found_devices = bc_onewire_search_all(onewire, device_list, sizeof(device_list));

    bc_scheduler_plan_current_relative(1000);
}
