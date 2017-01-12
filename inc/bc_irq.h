#ifndef _BC_IRQ_H
#define _BC_IRQ_H

#include "bc_common.h"

//! @addtogroup bc_irq bc_irq
//! @brief Functions to enable and disable global interrupts
//! @{

//! @brief Disable global IRQs

void bc_irq_disable(void);

//! @brief Enable global IRQs

void bc_irq_enable(void);

//! @}

#endif /* _BC_IRQ_H */
