#ifndef _TWR_IRQ_H
#define _TWR_IRQ_H

#include <twr_common.h>

//! @addtogroup twr_irq twr_irq
//! @brief Functions for interrupt request manipulation
//! @{

//! @brief Disable interrupt requests globally (call can be nested)

void twr_irq_disable(void);

//! @brief Enable interrupt requests globally (call can be nested)

void twr_irq_enable(void);

//! @}

#endif // _TWR_IRQ_H
