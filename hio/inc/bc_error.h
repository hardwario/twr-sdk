#ifndef _HIO_ERROR_H
#define _HIO_ERROR_H

#include <hio_common.h>

typedef enum
{
    HIO_ERROR_NOT_ENOUGH_TASKS = 0,
    HIO_ERROR_LOG_NOT_INITIALIZED = 1,
    HIO_ERROR_ERROR_UNLOCK = 2,
    HIO_ERROR_CALLBACK = 3,

} hio_error_t;

void hio_error(hio_error_t code);

#endif // _HIO_ERROR_H
