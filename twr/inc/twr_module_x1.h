#ifndef _TWR_MODULE_X1_H
#define _TWR_MODULE_X1_H

#include <twr_onewire.h>

//! @addtogroup twr_module_x1 twr_module_x1
//! @brief Driver for X1 Module
//! @{

//! @brief Initialize X1 Module
//! @return true On success
//! @return false On Error

bool twr_module_x1_init(void);

//! @brief Initialize and get Instance 1-Wire for channel B
//! @return pointer on Instance 1-Wire

twr_onewire_t *twr_module_x1_get_onewire(void);

//! @}

#endif // _TWR_MODULE_X1_H
