#include <hio_error.h>

void application_error(hio_error_t code);

void hio_error(hio_error_t code)
{
    application_error(code);
}
