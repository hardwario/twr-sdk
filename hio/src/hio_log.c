#include <hio_log.h>
#include <hio_error.h>

typedef struct
{
    bool initialized;
    hio_log_level_t level;
    hio_log_timestamp_t timestamp;
    hio_tick_t tick_last;

    char buffer[256];

} hio_log_t;

#ifndef RELEASE

static hio_log_t _hio_log = { .initialized = false };

void application_error(hio_error_t code);

static void _hio_log_message(hio_log_level_t level, char id, const char *format, va_list ap);

void hio_log_init(hio_log_level_t level, hio_log_timestamp_t timestamp)
{
    if (_hio_log.initialized)
    {
        return;
    }

    memset(&_hio_log, 0, sizeof(_hio_log));

    _hio_log.level = level;
    _hio_log.timestamp = timestamp;

    hio_uart_init(HIO_LOG_UART, HIO_UART_BAUDRATE_115200, HIO_UART_SETTING_8N1);
    hio_uart_write(HIO_LOG_UART, "\r\n", 2);

    _hio_log.initialized = true;
}

void hio_log_dump(const void *buffer, size_t length, const char *format, ...)
{
    va_list ap;

    if (_hio_log.level > HIO_LOG_LEVEL_DUMP)
    {
        return;
    }

    va_start(ap, format);
    _hio_log_message(HIO_LOG_LEVEL_DUMP, 'X', format, ap);
    va_end(ap);

    size_t offset_base = 0;

    for (; offset_base < sizeof(_hio_log.buffer); offset_base++)
    {
        if (_hio_log.buffer[offset_base] == '>')
        {
            break;
        }
    }

    offset_base += 2;

    size_t position;

    size_t offset;

    if (buffer != NULL && length != 0)
    {
        for (position = 0; position < length; position += HIO_LOG_DUMP_WIDTH)
        {
            offset = offset_base + snprintf(_hio_log.buffer + offset_base, sizeof(_hio_log.buffer) - offset_base, "%3d: ", position);

            char *ptr_hex = _hio_log.buffer + offset;

            offset += (HIO_LOG_DUMP_WIDTH * 3 + 2 + 1);

            char *ptr_text = _hio_log.buffer + offset;

            offset += HIO_LOG_DUMP_WIDTH;

            uint32_t line_size;

            uint32_t i;

            if ((position + HIO_LOG_DUMP_WIDTH) <= length)
            {
                line_size = HIO_LOG_DUMP_WIDTH;
            }
            else
            {
                line_size = length - position;
            }

            for (i = 0; i < line_size; i++)
            {
                uint8_t value = ((uint8_t *) buffer)[position + i];

                if (i == (HIO_LOG_DUMP_WIDTH / 2))
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

            for (; i < HIO_LOG_DUMP_WIDTH; i++)
            {
                if (i == (HIO_LOG_DUMP_WIDTH / 2))
                {
                    *ptr_hex++ = '|';
                    *ptr_hex++ = ' ';
                }

                strcpy(ptr_hex, "   ");

                ptr_hex += 3;

                *ptr_text++ = ' ';
            }

            _hio_log.buffer[offset++] = '\r';
            _hio_log.buffer[offset++] = '\n';

            hio_uart_write(HIO_LOG_UART, _hio_log.buffer, offset);
        }
    }
}

void hio_log_debug(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    _hio_log_message(HIO_LOG_LEVEL_DEBUG, 'D', format, ap);
    va_end(ap);
}

void hio_log_info(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    _hio_log_message(HIO_LOG_LEVEL_INFO, 'I', format, ap);
    va_end(ap);
}

void hio_log_warning(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    _hio_log_message(HIO_LOG_LEVEL_WARNING, 'W', format, ap);
    va_end(ap);
}

void hio_log_error(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    _hio_log_message(HIO_LOG_LEVEL_ERROR, 'E', format, ap);
    va_end(ap);
}

static void _hio_log_message(hio_log_level_t level, char id, const char *format, va_list ap)
{
    if (!_hio_log.initialized)
    {
        application_error(HIO_ERROR_LOG_NOT_INITIALIZED);
    }

    if (_hio_log.level > level)
    {
        return;
    }

    size_t offset;

    if (_hio_log.timestamp == HIO_LOG_TIMESTAMP_ABS)
    {
        hio_tick_t tick_now = hio_tick_get();

        uint32_t timestamp_abs = tick_now / 10;

        offset = sprintf(_hio_log.buffer, "# %lu.%02lu <%c> ", timestamp_abs / 100, timestamp_abs % 100, id);
    }
    else if (_hio_log.timestamp == HIO_LOG_TIMESTAMP_REL)
    {
        hio_tick_t tick_now = hio_tick_get();

        uint32_t timestamp_rel = (tick_now - _hio_log.tick_last) / 10;

        offset = sprintf(_hio_log.buffer, "# +%lu.%02lu <%c> ", timestamp_rel / 100, timestamp_rel % 100, id);

        _hio_log.tick_last = tick_now;
    }
    else
    {
        strcpy(_hio_log.buffer, "# <!> ");

        _hio_log.buffer[3] = id;

        offset = 6;
    }

    offset += vsnprintf(&_hio_log.buffer[offset], sizeof(_hio_log.buffer) - offset - 3, format, ap);

    _hio_log.buffer[offset++] = '\r';
    _hio_log.buffer[offset++] = '\n';

    hio_uart_write(HIO_LOG_UART, _hio_log.buffer, offset);
}

#endif
