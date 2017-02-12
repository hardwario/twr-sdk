#include <bc_device_id.h>
#include <stm32l0xx.h>

void bc_device_id_get(void *destination, size_t size)
{
    memset(destination, 0, size);
    memcpy(destination, (void *) UID_BASE, size > 12 ? 12 : size);
}
