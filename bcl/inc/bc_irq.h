#ifndef _BC_IRQ_H
#define _BC_IRQ_H

#include <bc_common.h>

//! @addtogroup bc_irq bc_irq
//! @brief Functions for interrupt request manipulation
//! @{

//! @brief Disable interrupt requests globally (call can be nested)

void bc_irq_disable(void);

//! @brief Enable interrupt requests globally (call can be nested)

void bc_irq_enable(void);

//! @}

#endif // _BC_IRQ_H
