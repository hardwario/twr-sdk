#ifndef _TWR_ONEWIRE_DS2484_H
#define _TWR_ONEWIRE_DS2484_H

#include <twr_onewire.h>
#include <twr_ds2484.h>

//! @addtogroup twr_onewire twr_onewire_gpio
//! @brief Driver for GPIO 1-Wire
//! @{

//! @brief Initialize 1-Wire
//! @param[in] onewire Instance 1-Wire

void twr_onewire_ds2484_init(twr_onewire_t *onewire, twr_ds2484_t *twr_ds2484);

//! @}

#endif // _TWR_ONEWIRE_GPIO_H
