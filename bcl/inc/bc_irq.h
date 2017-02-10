#ifndef _BC_IRQ_H
#define _BC_IRQ_H

#include <bc_common.h>

//! @addtogroup bc_irq bc_irq
//! @brief Functions to manipulate interrupt requests
//! @{

//! @brief Deny all interrupt requests

void bc_irq_disable(void);

//! @brief Allow all interrupt requests

void bc_irq_enable(void);

//! @}

#endif // _BC_IRQ_H
