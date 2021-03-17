#include <twr_atci.h>
#include <twr_scheduler.h>
#include <twr_system.h>

static void _twr_atci_uart_event_handler(twr_uart_channel_t channel, twr_uart_event_t event, void  *event_param);
static void _twr_atci_uart_active_test(void);
static void _twr_atci_uart_active_test_task(void  *param);

static struct
{
    const twr_atci_command_t *commands;
    size_t commands_length;
    char tx_buffer[256];
    char rx_buffer[256];
    size_t rx_length;
    bool rx_error;
    uint8_t read_fifo_buffer[128];
    twr_fifo_t read_fifo;
    twr_scheduler_task_id_t vbus_sense_test_task_id;
    bool ready;
    bool (*uart_active_callback)(void);
    twr_tick_t scan_interval;
    bool write_response;

} _twr_atci;

void twr_atci_init(const twr_atci_command_t *commands, int length)
{
    memset(&_twr_atci, 0, sizeof(_twr_atci));

    _twr_atci.commands = commands;

    _twr_atci.commands_length = length;

    _twr_atci.rx_length = 0;

    _twr_atci.rx_error = false;

    _twr_atci.write_response = true;

    twr_fifo_init(&_twr_atci.read_fifo, _twr_atci.read_fifo_buffer, sizeof(_twr_atci.read_fifo_buffer));

    twr_atci_set_uart_active_callback(twr_system_get_vbus_sense, 200);
}

size_t twr_atci_print(const char *message)
{
    return twr_uart_write(TWR_ATCI_UART, message, strlen(message));
}

size_t twr_atci_println(const char *message)
{
    size_t len = twr_uart_write(TWR_ATCI_UART, message, strlen(message));
    return twr_uart_write(TWR_ATCI_UART, "\r\n", 2) + len;
}

static size_t _twr_atci_printf(const char *format, va_list ap, size_t maxlen)
{
    size_t length = vsnprintf(_twr_atci.tx_buffer, maxlen, format, ap);

    if (length > maxlen) {
        length = maxlen;
    }

    return length;
}

size_t twr_atci_printf(const char *format, ...)
{
    if (!_twr_atci.ready)
    {
        return 0;
    }

    va_list ap;

    va_start(ap, format);
    size_t length = _twr_atci_printf(format, ap, sizeof(_twr_atci.tx_buffer));
    va_end(ap);

    return twr_uart_write(TWR_ATCI_UART, _twr_atci.tx_buffer, length);
}

size_t twr_atci_printfln(const char *format, ...)
{
    if (!_twr_atci.ready)
    {
        return 0;
    }

    va_list ap;

    va_start(ap, format);
    size_t length = _twr_atci_printf(format, ap, sizeof(_twr_atci.tx_buffer) - 2);
    va_end(ap);

    _twr_atci.tx_buffer[length++] = '\r';
    _twr_atci.tx_buffer[length++] = '\n';

    return twr_uart_write(TWR_ATCI_UART, _twr_atci.tx_buffer, length);
}

size_t twr_atci_print_buffer_as_hex(const void *buffer, size_t length)
{
    if (!_twr_atci.ready)
    {
        return 0;
    }

    char byte;
    size_t on_write = 0;

    for (size_t i = 0; i < length; i++)
    {
        byte = ((char *)buffer)[i];

        char upper = (byte >> 4) & 0xf;
        char lower = byte & 0x0f;

        _twr_atci.tx_buffer[on_write++] = upper < 10 ? upper + '0' : upper - 10 + 'A';
        _twr_atci.tx_buffer[on_write++] = lower < 10 ? lower + '0' : lower - 10 + 'A';
    }

    return twr_uart_write(TWR_ATCI_UART, _twr_atci.tx_buffer, on_write);
}

bool twr_atci_skip_response(void)
{
    _twr_atci.write_response = false;

    return true;
}

void twr_atci_write_ok(void)
{
    twr_uart_write(TWR_ATCI_UART, "OK\r\n", 4);
}

void twr_atci_write_error(void)
{
    twr_uart_write(TWR_ATCI_UART, "ERROR\r\n", 7);
}

bool twr_atci_clac_action(void)
{
    for (size_t i = 0; i < _twr_atci.commands_length; i++)
    {
        twr_atci_printfln("AT%s", _twr_atci.commands[i].command);
    }
    return true;
}

bool twr_atci_help_action(void)
{
    for (size_t i = 0; i < _twr_atci.commands_length; i++)
    {
        twr_atci_printfln("AT%s %s", _twr_atci.commands[i].command, _twr_atci.commands[i].hint);
    }
    return true;
}

static bool _twr_atci_process_line(void)
{
    if (_twr_atci.rx_length < 2 || _twr_atci.rx_buffer[0] != 'A' || _twr_atci.rx_buffer[1] != 'T')
    {
        return false;
    }

    if (_twr_atci.rx_length == 2)
    {
        return true;
    }

    _twr_atci.rx_buffer[_twr_atci.rx_length] = 0;

    char *line = _twr_atci.rx_buffer + 2;

    size_t length = _twr_atci.rx_length - 2;

    size_t command_len;

    const twr_atci_command_t *command;

    for (size_t i = 0; i < _twr_atci.commands_length; i++)
    {
        command = _twr_atci.commands + i;

        command_len = strlen(command->command);

        if (length < command_len)
        {
            continue;
        }

        if (strncmp(line, command->command, command_len) != 0)
        {
            continue;
        }

        if (command_len == length)
        {
            if (command->action != NULL)
            {
                return command->action();
            }
        }
        else if (line[command_len] == '=')
        {
            if ((line[command_len + 1]) == '?' && (command_len + 2 == length))
            {
                if (command->help != NULL)
                {
                    return command->help();
                }
            }

            if (command->set != NULL)
            {
                twr_atci_param_t param = {
                        .txt = line + command_len + 1,
                        .length = length - command_len - 1,
                        .offset = 0
                };

                return command->set(&param);
            }
        }
        else if (line[command_len] == '?' && command_len + 1 == length)
        {
            if (command->read != NULL)
            {
                return command->read();
            }
        }
        else
        {
            return false;
        }

        break;
    }

    return false;
}

static void _twr_atci_process_character(char character)
{
    if (character == '\n')
    {
        if (!_twr_atci.rx_error && _twr_atci.rx_length > 0)
        {
            bool response = _twr_atci_process_line();

            if (_twr_atci.write_response)
            {
                if (response)
                {
                    twr_atci_write_ok();
                }
                else
                {
                    twr_atci_write_error();
                }
            }
            else
            {
                _twr_atci.write_response = true;
            }
        }
        else if (_twr_atci.rx_error)
        {
            twr_atci_write_error();
        }

        _twr_atci.rx_length = 0;
        _twr_atci.rx_error = false;
    }
    else if (character == '\r')
    {
        return;
    }
    else if (character == '\x1b')
    {
        _twr_atci.rx_length = 0;
        _twr_atci.rx_error = false;
    }
    else if (_twr_atci.rx_length == sizeof(_twr_atci.rx_buffer) - 1)
    {
        _twr_atci.rx_error = true;
    }
    else if (!_twr_atci.rx_error)
    {
        _twr_atci.rx_buffer[_twr_atci.rx_length++] = character;
    }
}

static void _twr_atci_uart_event_handler(twr_uart_channel_t channel, twr_uart_event_t event, void  *event_param)
{
    (void) channel;
    (void) event_param;

    if (event == TWR_UART_EVENT_ASYNC_READ_DATA)
    {
        while (true)
        {
            static uint8_t buffer[16];

            size_t length = twr_uart_async_read(TWR_ATCI_UART, buffer, sizeof(buffer));

            if (length == 0)
            {
                break;
            }

            for (size_t i = 0; i < length; i++)
            {
                _twr_atci_process_character((char) buffer[i]);
            }
        }
    }
}

bool twr_atci_get_uint(twr_atci_param_t *param, uint32_t *value)
{
    char c;

    *value = 0;

    while (param->offset < param->length)
    {
        c = param->txt[param->offset];

        if (isdigit(c))
        {
            *value *= 10;
            *value += c - '0';
        }
        else
        {
            if (c == ',')
            {
                return true;
            }
            return false;
        }

        param->offset++;
    }

    return true;
}

bool twr_atci_get_string(twr_atci_param_t *param, char *str, size_t length)
{
    if (((param->length - param->offset) < 2) || (length < 1) || (str == NULL))
    {
        return false;
    }

    if (param->txt[param->offset++] != '"')
    {
        return false;
    }

    char c;
    size_t i;

    for (i = 0; (i < length) && (param->offset < param->length); i++)
    {
        c = param->txt[param->offset++];

        if (c == '"')
        {
            str[i] = 0;

            return true;
        }

        if ((c < ' ') || (c == ',') || (c > '~'))
        {
            return false;
        }

        str[i] = c;
    }

    return false;
}

bool twr_atci_get_buffer_from_hex_string(twr_atci_param_t *param, void *buffer, size_t *length)
{
    if (((param->length - param->offset) < 2) || (*length < 1) || (buffer == NULL))
    {
        return false;
    }

    if (param->txt[param->offset++] != '"')
    {
        return false;
    }

    char c;
    size_t i;
    size_t max_i = *length * 2;
    uint8_t temp;
    size_t l = 0;

    for (i = 0; (i < max_i) && (param->offset < param->length); i++)
    {
        c = param->txt[param->offset++];

        if (c == '"')
        {
            *length = l;

            return true;
        }

        if ((c >= '0') && (c <= '9'))
        {
            temp = c - '0';
        }
        else if ((c >= 'A') && (c <= 'F'))
        {
            temp = c - 'A' + 10;
        }
        else if ((c >= 'a') && (c <= 'f'))
        {
            temp = c - 'a' + 10;
        }
        else
        {
            return false;
        }

        if (i % 2 == 0)
        {
            if (l == *length)
            {
                return false;
            }

            ((uint8_t *) buffer)[l] = temp << 4;
        }
        else
        {
            ((uint8_t *) buffer)[l++] |= temp;
        }
    }

    return false;
}

bool twr_atci_is_comma(twr_atci_param_t *param)
{
    return param->txt[param->offset++] == ',';
}

bool twr_atci_is_quotation_mark(twr_atci_param_t *param)
{
    return param->txt[param->offset++] == '"';
}

void twr_atci_set_uart_active_callback(bool(*callback)(void), twr_tick_t scan_interval)
{
    _twr_atci.uart_active_callback = callback;
    _twr_atci.scan_interval = scan_interval;

    if (callback == NULL)
    {
        if (_twr_atci.vbus_sense_test_task_id)
        {
            twr_scheduler_unregister(_twr_atci.vbus_sense_test_task_id);

            _twr_atci.vbus_sense_test_task_id = 0;
        }
    }
    else
    {
        if (_twr_atci.vbus_sense_test_task_id == 0)
        {
            _twr_atci.vbus_sense_test_task_id = twr_scheduler_register(_twr_atci_uart_active_test_task, NULL, scan_interval);
        }
    }

    _twr_atci_uart_active_test();
}

static void _twr_atci_uart_active_test(void)
{
    if ((_twr_atci.uart_active_callback == NULL) || _twr_atci.uart_active_callback())
    {
        if (!_twr_atci.ready)
        {
            twr_uart_init(TWR_ATCI_UART, TWR_UART_BAUDRATE_115200, TWR_UART_SETTING_8N1);

            twr_uart_set_async_fifo(TWR_ATCI_UART, NULL, &_twr_atci.read_fifo);

            twr_uart_set_event_handler(TWR_ATCI_UART, _twr_atci_uart_event_handler, NULL);

            twr_uart_async_read_start(TWR_ATCI_UART, 1000);

            _twr_atci.ready = true;

            twr_uart_write(TWR_ATCI_UART, "\r\n", 2);
        }
    }
    else
    {
        if (_twr_atci.ready)
        {
            _twr_atci.ready = false;

            twr_uart_deinit(TWR_ATCI_UART);
        }
    }
}

static void _twr_atci_uart_active_test_task(void *param)
{
    (void) param;

    _twr_atci_uart_active_test();

    twr_scheduler_plan_current_relative(_twr_atci.scan_interval);
}
