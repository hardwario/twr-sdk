#ifndef _HIO_IRQ_H
#define _HIO_IRQ_H

#include <hio_common.h>

//! @addtogroup hio_irq hio_irq
//! @brief Functions for interrupt request manipulation
//! @{

//! @brief Disable interrupt requests globally (call can be nested)

void hio_irq_disable(void);

//! @brief Enable interrupt requests globally (call can be nested)

void hio_irq_enable(void);

//! @}

#endif // _HIO_IRQ_H
