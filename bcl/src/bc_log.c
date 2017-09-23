#include <bc_log.h>
#include <bc_uart.h>

typedef struct
{
    bc_log_level_t level;
    bc_log_timestamp_t timestamp;
    bc_tick_t tick_last;

    char buffer[256];

} bc_log_t;

#ifndef RELEASE

static bc_log_t _bc_log;

static void _bc_log_message(bc_log_level_t level, char id, const char *format, va_list ap);

void bc_log_init(bc_log_level_t level, bc_log_timestamp_t timestamp)
{
    memset(&_bc_log, 0, sizeof(_bc_log));

    _bc_log.level = level;
    _bc_log.timestamp = timestamp;

    bc_uart_init(BC_UART_UART2, BC_UART_BAUDRATE_115200, BC_UART_SETTING_8N1);
    bc_uart_write(BC_UART_UART2, "\r\n", 2);
}

void bc_log_debug(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    _bc_log_message(BC_LOG_LEVEL_DEBUG, 'D', format, ap);
    va_end(ap);
}

void bc_log_info(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    _bc_log_message(BC_LOG_LEVEL_INFO, 'I', format, ap);
    va_end(ap);
}

void bc_log_warning(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    _bc_log_message(BC_LOG_LEVEL_WARNING, 'W', format, ap);
    va_end(ap);
}

void bc_log_error(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    _bc_log_message(BC_LOG_LEVEL_ERROR, 'E', format, ap);
    va_end(ap);
}

static void _bc_log_message(bc_log_level_t level, char id, const char *format, va_list ap)
{
    if (_bc_log.level == BC_LOG_LEVEL_OFF || _bc_log.level > level)
    {
        return;
    }

    bc_tick_t tick_now = bc_tick_get();

    if (_bc_log.timestamp == BC_LOG_TIMESTAMP_ABS)
    {
        uint32_t timestamp_abs = tick_now / 10;

        sprintf(_bc_log.buffer, "# %lu.%02lu <%c> ", timestamp_abs / 100, timestamp_abs % 100, id);
    }
    else if (_bc_log.timestamp == BC_LOG_TIMESTAMP_REL)
    {
        uint32_t timestamp_rel = (tick_now - _bc_log.tick_last) / 10;

        sprintf(_bc_log.buffer, "# +%lu.%02lu <%c> ", timestamp_rel / 100, timestamp_rel % 100, id);
    }
    else
    {
        strcpy(_bc_log.buffer, "# <!> ");

        _bc_log.buffer[3] = id;
    }

    _bc_log.tick_last = tick_now;

    size_t offset = strlen(_bc_log.buffer);

    vsnprintf(&_bc_log.buffer[offset], sizeof(_bc_log.buffer) - offset - 3, format, ap);

    strcat(_bc_log.buffer, "\r\n");

    bc_uart_write(BC_UART_UART2, _bc_log.buffer, strlen(_bc_log.buffer));
}

#endif
