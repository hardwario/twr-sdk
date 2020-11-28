#ifndef _HIO_ONEWIRE_GPIO_H
#define _HIO_ONEWIRE_GPIO_H

#include <hio_onewire.h>
#include <hio_gpio.h>

//! @addtogroup hio_onewire hio_onewire_gpio
//! @brief Driver for GPIO 1-Wire
//! @{

//! @brief Initialize 1-Wire
//! @param[in] onewire Instance 1-Wire
//! @param[in] gpio_channel GPIO channel
void hio_onewire_gpio_init(hio_onewire_t *onewire, hio_gpio_channel_t channel);

const hio_onewire_driver_t *hio_onewire_gpio_det_driver(void);

//! @}

#endif // _HIO_ONEWIRE_GPIO_H
