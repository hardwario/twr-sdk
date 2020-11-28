#ifndef _BC_ONEWIRE_DS2484_H
#define _BC_ONEWIRE_DS2484_H

#include <bc_onewire.h>
#include <bc_ds2484.h>

//! @addtogroup bc_onewire bc_onewire_gpio
//! @brief Driver for GPIO 1-Wire
//! @{

//! @brief Initialize 1-Wire
//! @param[in] onewire Instance 1-Wire

void bc_onewire_ds2484_init(bc_onewire_t *onewire, bc_ds2484_t *bc_ds2484);

//! @}

#endif // _BC_ONEWIRE_GPIO_H
