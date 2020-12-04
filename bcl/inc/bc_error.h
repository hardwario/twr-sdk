#ifndef _BC_ERROR_H
#define _BC_ERROR_H

#include <bc_common.h>

typedef enum
{
    BC_ERROR_NOT_ENOUGH_TASKS = 0,
    BC_ERROR_LOG_NOT_INITIALIZED = 1,
    BC_ERROR_ERROR_UNLOCK = 2,
    BC_ERROR_CALLBACK = 3,

} bc_error_t;

void bc_error(bc_error_t code);

#endif // _BC_ERROR_H
