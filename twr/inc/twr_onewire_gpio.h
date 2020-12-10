#ifndef _TWR_ONEWIRE_GPIO_H
#define _TWR_ONEWIRE_GPIO_H

#include <twr_onewire.h>
#include <twr_gpio.h>

//! @addtogroup twr_onewire twr_onewire_gpio
//! @brief Driver for GPIO 1-Wire
//! @{

//! @brief Initialize 1-Wire
//! @param[in] onewire Instance 1-Wire
//! @param[in] gpio_channel GPIO channel
void twr_onewire_gpio_init(twr_onewire_t *onewire, twr_gpio_channel_t channel);

const twr_onewire_driver_t *twr_onewire_gpio_det_driver(void);

//! @}

#endif // _TWR_ONEWIRE_GPIO_H
