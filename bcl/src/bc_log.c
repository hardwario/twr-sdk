#include <bc_log.h>
#include <bc_uart.h>

static struct
{
    bc_log_level_t level;

    char buffer[256];

} _bc_log;

void bc_log_init(bc_log_level_t level)
{
    memset(&_bc_log, 0, sizeof(_bc_log));

    _bc_log.level = level;

    bc_uart_init(BC_UART_UART2, BC_UART_BAUDRATE_115200, BC_UART_SETTING_8N1);
}

void bc_log_debug(const char *format, ...)
{
    va_list ap;

    if (_bc_log.level == BC_LOG_LEVEL_OFF || _bc_log.level > BC_LOG_LEVEL_DEBUG)
    {
        return;
    }

    strcpy(_bc_log.buffer, "# <D> ");

    va_start(ap, format);
    vsnprintf(&_bc_log.buffer[6], sizeof(_bc_log.buffer) - 6 - 2, format, ap);
    va_end(ap);

    strcat(_bc_log.buffer, "\r\n");

    bc_uart_write(BC_UART_UART2, _bc_log.buffer, strlen(_bc_log.buffer));
}

void bc_log_info(const char *format, ...)
{
    va_list ap;

    if (_bc_log.level == BC_LOG_LEVEL_OFF || _bc_log.level > BC_LOG_LEVEL_INFO)
    {
        return;
    }

    strcpy(_bc_log.buffer, "# <I> ");

    va_start(ap, format);
    vsnprintf(&_bc_log.buffer[6], sizeof(_bc_log.buffer) - 6 - 2, format, ap);
    va_end(ap);

    strcat(_bc_log.buffer, "\r\n");

    bc_uart_write(BC_UART_UART2, _bc_log.buffer, strlen(_bc_log.buffer));
}

void bc_log_warning(const char *format, ...)
{
    va_list ap;

    if (_bc_log.level == BC_LOG_LEVEL_OFF || _bc_log.level > BC_LOG_LEVEL_WARNING)
    {
        return;
    }

    strcpy(_bc_log.buffer, "# <W> ");

    va_start(ap, format);
    vsnprintf(&_bc_log.buffer[6], sizeof(_bc_log.buffer) - 6 - 2, format, ap);
    va_end(ap);

    strcat(_bc_log.buffer, "\r\n");

    bc_uart_write(BC_UART_UART2, _bc_log.buffer, strlen(_bc_log.buffer));
}

void bc_log_error(const char *format, ...)
{
    va_list ap;

    if (_bc_log.level == BC_LOG_LEVEL_OFF || _bc_log.level > BC_LOG_LEVEL_ERROR)
    {
        return;
    }

    strcpy(_bc_log.buffer, "# <E> ");

    va_start(ap, format);
    vsnprintf(&_bc_log.buffer[6], sizeof(_bc_log.buffer) - 6 - 2, format, ap);
    va_end(ap);

    strcat(_bc_log.buffer, "\r\n");

    bc_uart_write(BC_UART_UART2, _bc_log.buffer, strlen(_bc_log.buffer));
}
