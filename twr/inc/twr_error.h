#ifndef _TWR_ERROR_H
#define _TWR_ERROR_H

#include <twr_common.h>

typedef enum
{
    TWR_ERROR_NOT_ENOUGH_TASKS = 0,
    TWR_ERROR_LOG_NOT_INITIALIZED = 1,
    TWR_ERROR_ERROR_UNLOCK = 2,
    TWR_ERROR_CALLBACK = 3,

} twr_error_t;

void twr_error(twr_error_t code);

#endif // _TWR_ERROR_H
