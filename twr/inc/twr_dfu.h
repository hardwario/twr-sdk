#ifndef _TWR_DFU_H
#define _TWR_DFU_H

#include <twr_common.h>

//! @addtogroup twr_dfu twr_dfu
//! @brief USB DFU Bootloader function
//! @{

//! @brief Reset the CPU and jump to the USB DFU bootloader.
void twr_dfu_jump(void);

//! @}

#endif // _TWR_DFU_H
