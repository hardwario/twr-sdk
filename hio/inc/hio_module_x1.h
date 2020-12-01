#ifndef _HIO_MODULE_X1_H
#define _HIO_MODULE_X1_H

#include <hio_onewire.h>

//! @addtogroup hio_module_x1 hio_module_x1
//! @brief Driver for X1 Module
//! @{

//! @brief Initialize X1 Module
//! @return true On success
//! @return false On Error

bool hio_module_x1_init(void);

//! @brief Initialize and get Instance 1-Wire for channel B
//! @return pointer on Instance 1-Wire

hio_onewire_t *hio_module_x1_get_onewire(void);

//! @}

#endif // _HIO_MODULE_X1_H
