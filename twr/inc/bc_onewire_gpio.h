#ifndef _BC_ONEWIRE_GPIO_H
#define _BC_ONEWIRE_GPIO_H

#include <bc_onewire.h>
#include <bc_gpio.h>

//! @addtogroup bc_onewire bc_onewire_gpio
//! @brief Driver for GPIO 1-Wire
//! @{

//! @brief Initialize 1-Wire
//! @param[in] onewire Instance 1-Wire
//! @param[in] gpio_channel GPIO channel
void bc_onewire_gpio_init(bc_onewire_t *onewire, bc_gpio_channel_t channel);

const bc_onewire_driver_t *bc_onewire_gpio_det_driver(void);

//! @}

#endif // _BC_ONEWIRE_GPIO_H
