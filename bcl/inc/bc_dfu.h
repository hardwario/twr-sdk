#ifndef _BC_DFU_H
#define _BC_DFU_H

#include <bc_common.h>

//! @addtogroup bc_dfu bc_dfu
//! @brief USB DFU Bootloader function
//! @{

//! @brief Reset the CPU and jump to the USB DFU bootloader.
void bc_dfu_jump(void);

//! @}

#endif // _BC_DFU_H
