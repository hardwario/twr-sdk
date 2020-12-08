#ifndef _TWR_DEVICE_ID_H
#define _TWR_DEVICE_ID_H

#include <twr_common.h>

//! @addtogroup twr_device_id twr_device_id
//! @brief Functions for device unique ID
//! @{

//! @brief Get device unique ID
//! @param[out] destination Pointer to destination object where device unique ID will be stored
//! @param[in] size Size of destination object (in bytes)

void twr_device_id_get(void *destination, size_t size);

//! @}

#endif // _TWR_DEVICE_ID_H
