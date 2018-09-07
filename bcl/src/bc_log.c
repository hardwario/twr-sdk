#include <bc_log.h>
#include <bc_error.h>

typedef struct
{
    bool initialized;
    bc_log_level_t level;
    bc_log_timestamp_t timestamp;
    bc_tick_t tick_last;

    char buffer[256];

} bc_log_t;

#ifndef RELEASE

static bc_log_t _bc_log = { .initialized = false };

void application_error(bc_error_t code);

static void _bc_log_message(bc_log_level_t level, char id, const char *format, va_list ap);

void bc_log_init(bc_log_level_t level, bc_log_timestamp_t timestamp)
{
    if (_bc_log.initialized)
    {
        return;
    }

    memset(&_bc_log, 0, sizeof(_bc_log));

    _bc_log.level = level;
    _bc_log.timestamp = timestamp;

    bc_uart_init(BC_LOG_UART, BC_UART_BAUDRATE_115200, BC_UART_SETTING_8N1);
    bc_uart_write(BC_LOG_UART, "\r\n", 2);

    _bc_log.initialized = true;
}

void bc_log_dump(const void *buffer, size_t length, const char *format, ...)
{
    va_list ap;

    if (_bc_log.level > BC_LOG_LEVEL_DUMP)
    {
        return;
    }

    va_start(ap, format);
    _bc_log_message(BC_LOG_LEVEL_DUMP, 'X', format, ap);
    va_end(ap);

    size_t offset_base = 0;

    for (; offset_base < sizeof(_bc_log.buffer); offset_base++)
    {
        if (_bc_log.buffer[offset_base] == '>')
        {
            break;
        }
    }

    offset_base += 2;

    size_t position;

    size_t offset;

    if (buffer != NULL && length != 0)
    {
        for (position = 0; position < length; position += BC_LOG_DUMP_WIDTH)
        {
            offset = offset_base + snprintf(_bc_log.buffer + offset_base, sizeof(_bc_log.buffer) - offset_base, "%3d: ", position);

            char *ptr_hex = _bc_log.buffer + offset;

            offset += (BC_LOG_DUMP_WIDTH * 3 + 2 + 1);

            char *ptr_text = _bc_log.buffer + offset;

            offset += BC_LOG_DUMP_WIDTH;

            uint32_t line_size;

            uint32_t i;

            if ((position + BC_LOG_DUMP_WIDTH) <= length)
            {
                line_size = BC_LOG_DUMP_WIDTH;
            }
            else
            {
                line_size = length - position;
            }

            for (i = 0; i < line_size; i++)
            {
                uint8_t value = ((uint8_t *) buffer)[position + i];

                if (i == (BC_LOG_DUMP_WIDTH / 2))
                {
                    *ptr_hex++ = '|';
                    *ptr_hex++ = ' ';
                }

                snprintf(ptr_hex, 4, "%02X ", value);

                ptr_hex += 3;

                if (value < 32 || value > 126)
                {
                    *ptr_text++ = '.';
                }
                else
                {
                    *ptr_text++ = value;
                }
            }

            for (; i < BC_LOG_DUMP_WIDTH; i++)
            {
                if (i == (BC_LOG_DUMP_WIDTH / 2))
                {
                    *ptr_hex++ = '|';
                    *ptr_hex++ = ' ';
                }

                strcpy(ptr_hex, "   ");

                ptr_hex += 3;

                *ptr_text++ = ' ';
            }

            _bc_log.buffer[offset++] = '\r';
            _bc_log.buffer[offset++] = '\n';

            bc_uart_write(BC_LOG_UART, _bc_log.buffer, offset);
        }
    }
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
    if (!_bc_log.initialized)
    {
        application_error(BC_ERROR_LOG_NOT_INITIALIZED);
    }

    if (_bc_log.level > level)
    {
        return;
    }

    size_t offset;

    if (_bc_log.timestamp == BC_LOG_TIMESTAMP_ABS)
    {
        bc_tick_t tick_now = bc_tick_get();

        uint32_t timestamp_abs = tick_now / 10;

        offset = sprintf(_bc_log.buffer, "# %lu.%02lu <%c> ", timestamp_abs / 100, timestamp_abs % 100, id);
    }
    else if (_bc_log.timestamp == BC_LOG_TIMESTAMP_REL)
    {
        bc_tick_t tick_now = bc_tick_get();

        uint32_t timestamp_rel = (tick_now - _bc_log.tick_last) / 10;

        offset = sprintf(_bc_log.buffer, "# +%lu.%02lu <%c> ", timestamp_rel / 100, timestamp_rel % 100, id);

        _bc_log.tick_last = tick_now;
    }
    else
    {
        strcpy(_bc_log.buffer, "# <!> ");

        _bc_log.buffer[3] = id;

        offset = 6;
    }

    offset += vsnprintf(&_bc_log.buffer[offset], sizeof(_bc_log.buffer) - offset - 3, format, ap);

    _bc_log.buffer[offset++] = '\r';
    _bc_log.buffer[offset++] = '\n';

    bc_uart_write(BC_LOG_UART, _bc_log.buffer, offset);
}

#endif
