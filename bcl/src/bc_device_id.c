#include <bc_device_id.h>
#include <stm32l0xx.h>

void bc_device_id_get(void *destination, size_t size)
{
    memset(destination, 0, size);
    memcpy(destination, (void *) (UID_BASE + (12 - (size > 12 ? 12 : size))), size > 12 ? 12 : size);
}
