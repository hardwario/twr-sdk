#ifndef _HIO_ONEWIRE_DS2484_H
#define _HIO_ONEWIRE_DS2484_H

#include <hio_onewire.h>
#include <hio_ds2484.h>

//! @addtogroup hio_onewire hio_onewire_gpio
//! @brief Driver for GPIO 1-Wire
//! @{

//! @brief Initialize 1-Wire
//! @param[in] onewire Instance 1-Wire

void hio_onewire_ds2484_init(hio_onewire_t *onewire, hio_ds2484_t *hio_ds2484);

//! @}

#endif // _HIO_ONEWIRE_GPIO_H
