#ifndef _HIO_DFU_H
#define _HIO_DFU_H

#include <hio_common.h>

//! @addtogroup hio_dfu hio_dfu
//! @brief USB DFU Bootloader function
//! @{

//! @brief Reset the CPU and jump to the USB DFU bootloader.
void hio_dfu_jump(void);

//! @}

#endif // _HIO_DFU_H
