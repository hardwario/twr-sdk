#ifndef _BC_DEVICE_ID_H
#define _BC_DEVICE_ID_H

#include <bc_common.h>

//! @addtogroup bc_device_id bc_device_id
//! @brief Functions for device unique ID
//! @{

//! @brief Get device unique ID
//! @param[out] destination Pointer to destination object where device unique ID will be stored
//! @param[in] size Size of destination object (in bytes)

void bc_device_id_get(void *destination, size_t size);

//! @}

#endif // _BC_DEVICE_ID_H
