#include <twr_log.h>
#include <twr_error.h>

typedef struct
{
    bool initialized;
    twr_log_level_t level;
    twr_log_timestamp_t timestamp;
    twr_tick_t tick_last;
    char buffer[TWR_LOG_BUFFER_SIZE];

} twr_log_t;

#ifndef RELEASE

static twr_log_t _twr_log = { .initialized = false };

void application_error(twr_error_t code);

static void _twr_log_message(twr_log_level_t level, char id, const char *format, va_list ap);

void twr_log_init(twr_log_level_t level, twr_log_timestamp_t timestamp)
{
    if (_twr_log.initialized)
    {
        return;
    }

    memset(&_twr_log, 0, sizeof(_twr_log));

    _twr_log.level = level;
    _twr_log.timestamp = timestamp;

    twr_uart_init(TWR_LOG_UART, TWR_UART_BAUDRATE_115200, TWR_UART_SETTING_8N1);
    twr_uart_write(TWR_LOG_UART, "\r\n", 2);

    _twr_log.initialized = true;
}

void twr_log_dump(const void *buffer, size_t length, const char *format, ...)
{
    va_list ap;

    if (_twr_log.level > TWR_LOG_LEVEL_DUMP)
    {
        return;
    }

    va_start(ap, format);
    _twr_log_message(TWR_LOG_LEVEL_DUMP, 'X', format, ap);
    va_end(ap);

    size_t offset_base = 0;

    for (; offset_base < sizeof(_twr_log.buffer); offset_base++)
    {
        if (_twr_log.buffer[offset_base] == '>')
        {
            break;
        }
    }

    offset_base += 2;

    size_t position;

    size_t offset;

    if (buffer != NULL && length != 0)
    {
        for (position = 0; position < length; position += TWR_LOG_DUMP_WIDTH)
        {
            offset = offset_base + snprintf(_twr_log.buffer + offset_base, sizeof(_twr_log.buffer) - offset_base, "%3d: ", position);

            char *ptr_hex = _twr_log.buffer + offset;

            offset += (TWR_LOG_DUMP_WIDTH * 3 + 2 + 1);

            char *ptr_text = _twr_log.buffer + offset;

            offset += TWR_LOG_DUMP_WIDTH;

            uint32_t line_size;

            uint32_t i;

            if ((position + TWR_LOG_DUMP_WIDTH) <= length)
            {
                line_size = TWR_LOG_DUMP_WIDTH;
            }
            else
            {
                line_size = length - position;
            }

            for (i = 0; i < line_size; i++)
            {
                uint8_t value = ((uint8_t *) buffer)[position + i];

                if (i == (TWR_LOG_DUMP_WIDTH / 2))
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

            for (; i < TWR_LOG_DUMP_WIDTH; i++)
            {
                if (i == (TWR_LOG_DUMP_WIDTH / 2))
                {
                    *ptr_hex++ = '|';
                    *ptr_hex++ = ' ';
                }

                strcpy(ptr_hex, "   ");

                ptr_hex += 3;

                *ptr_text++ = ' ';
            }

            _twr_log.buffer[offset++] = '\r';
            _twr_log.buffer[offset++] = '\n';

            twr_uart_write(TWR_LOG_UART, _twr_log.buffer, offset);
        }
    }
}

void twr_log_debug(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    _twr_log_message(TWR_LOG_LEVEL_DEBUG, 'D', format, ap);
    va_end(ap);
}

void twr_log_info(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    _twr_log_message(TWR_LOG_LEVEL_INFO, 'I', format, ap);
    va_end(ap);
}

void twr_log_warning(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    _twr_log_message(TWR_LOG_LEVEL_WARNING, 'W', format, ap);
    va_end(ap);
}

void twr_log_error(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    _twr_log_message(TWR_LOG_LEVEL_ERROR, 'E', format, ap);
    va_end(ap);
}

static void _twr_log_message(twr_log_level_t level, char id, const char *format, va_list ap)
{
    if (!_twr_log.initialized)
    {
        application_error(TWR_ERROR_LOG_NOT_INITIALIZED);
    }

    if (_twr_log.level > level)
    {
        return;
    }

    size_t offset;

    if (_twr_log.timestamp == TWR_LOG_TIMESTAMP_ABS)
    {
        twr_tick_t tick_now = twr_tick_get();

        uint32_t timestamp_abs = tick_now / 10;

        offset = sprintf(_twr_log.buffer, "# %lu.%02lu <%c> ", timestamp_abs / 100, timestamp_abs % 100, id);
    }
    else if (_twr_log.timestamp == TWR_LOG_TIMESTAMP_REL)
    {
        twr_tick_t tick_now = twr_tick_get();

        uint32_t timestamp_rel = (tick_now - _twr_log.tick_last) / 10;

        offset = sprintf(_twr_log.buffer, "# +%lu.%02lu <%c> ", timestamp_rel / 100, timestamp_rel % 100, id);

        _twr_log.tick_last = tick_now;
    }
    else
    {
        strcpy(_twr_log.buffer, "# <!> ");

        _twr_log.buffer[3] = id;

        offset = 6;
    }

    offset += vsnprintf(&_twr_log.buffer[offset], sizeof(_twr_log.buffer) - offset - 1, format, ap);

    if (offset >= sizeof(_twr_log.buffer))
    {
        offset = sizeof(_twr_log.buffer) - 3 - 3; // space for ...\r\n
        _twr_log.buffer[offset++] = '.';
        _twr_log.buffer[offset++] = '.';
        _twr_log.buffer[offset++] = '.';
    }

    _twr_log.buffer[offset++] = '\r';
    _twr_log.buffer[offset++] = '\n';

    twr_uart_write(TWR_LOG_UART, _twr_log.buffer, offset);
}

#endif
