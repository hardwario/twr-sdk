#include <application.h>
#include <bc_onewire.h>

int found_devices;
uint64_t device_list[5];

void application_init(void)
{

    bc_module_sensor_init();

    bc_module_sensor_set_pull(BC_MODULE_SENSOR_CHANNEL_A, BC_MODULE_SENSOR_PULL_UP_4K7);
    bc_onewire_init(BC_GPIO_P4);

    bc_module_sensor_set_pull(BC_MODULE_SENSOR_CHANNEL_B, BC_MODULE_SENSOR_PULL_UP_4K7);
    bc_onewire_init(BC_GPIO_P5);

}

void application_task()
{

    // Search on channel A
    memset(device_list, 0x00, sizeof(device_list));
    found_devices = bc_onewire_search_all(BC_GPIO_P4, device_list, sizeof(device_list));

    // Search on channel B
    memset(device_list, 0x00, sizeof(device_list));
    found_devices = bc_onewire_search_all(BC_GPIO_P5, device_list, sizeof(device_list));

    bc_scheduler_plan_current_relative(1000);
}
