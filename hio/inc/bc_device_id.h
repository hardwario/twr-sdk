#ifndef _HIO_DEVICE_ID_H
#define _HIO_DEVICE_ID_H

#include <hio_common.h>

//! @addtogroup hio_device_id hio_device_id
//! @brief Functions for device unique ID
//! @{

//! @brief Get device unique ID
//! @param[out] destination Pointer to destination object where device unique ID will be stored
//! @param[in] size Size of destination object (in bytes)

void hio_device_id_get(void *destination, size_t size);

//! @}

#endif // _HIO_DEVICE_ID_H
