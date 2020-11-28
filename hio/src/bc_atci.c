#include <hio_atci.h>
#include <hio_scheduler.h>
#include <hio_system.h>

static void _hio_atci_uart_event_handler(hio_uart_channel_t channel, hio_uart_event_t event, void  *event_param);
static void _hio_atci_uart_active_test(void);
static void _hio_atci_uart_active_test_task(void  *param);

static struct
{
    const hio_atci_command_t *commands;
    size_t commands_length;
    char tx_buffer[256];
    char rx_buffer[256];
    size_t rx_length;
    bool rx_error;
    uint8_t read_fifo_buffer[128];
    hio_fifo_t read_fifo;
    hio_scheduler_task_id_t vbus_sense_test_task_id;
    bool ready;
    bool (*uart_active_callback)(void);
    hio_tick_t scan_interval;
    bool write_response;

} _hio_atci;

void hio_atci_init(const hio_atci_command_t *commands, int length)
{
    memset(&_hio_atci, 0, sizeof(_hio_atci));

    _hio_atci.commands = commands;

    _hio_atci.commands_length = length;

    _hio_atci.rx_length = 0;

    _hio_atci.rx_error = false;

    _hio_atci.write_response = true;

    hio_fifo_init(&_hio_atci.read_fifo, _hio_atci.read_fifo_buffer, sizeof(_hio_atci.read_fifo_buffer));

    hio_atci_set_uart_active_callback(hio_system_get_vbus_sense, 200);
}

void hio_atci_printf(const char *format, ...)
{
    if (!_hio_atci.ready)
    {
        return;
    }

    va_list ap;

    size_t length;

    va_start(ap, format);

    length = vsnprintf(_hio_atci.tx_buffer, sizeof(_hio_atci.tx_buffer) - 2, format, ap);

    va_end(ap);

    _hio_atci.tx_buffer[length++] = '\r';
    _hio_atci.tx_buffer[length++] = '\n';

    hio_uart_write(HIO_ATCI_UART, _hio_atci.tx_buffer, length);
}

bool hio_atci_skip_response(void)
{
    _hio_atci.write_response = false;

    return true;
}

void hio_atci_write_ok(void)
{
    hio_uart_write(HIO_ATCI_UART, "OK\r\n", 4);
}

void hio_atci_write_error(void)
{
    hio_uart_write(HIO_ATCI_UART, "ERROR\r\n", 7);
}

bool hio_atci_clac_action(void)
{
    for (size_t i = 0; i < _hio_atci.commands_length; i++)
    {
        hio_atci_printf("AT%s", _hio_atci.commands[i].command);
    }
    return true;
}

bool hio_atci_help_action(void)
{
    for (size_t i = 0; i < _hio_atci.commands_length; i++)
    {
        hio_atci_printf("AT%s %s", _hio_atci.commands[i].command, _hio_atci.commands[i].hint);
    }
    return true;
}

static bool _hio_atci_process_line(void)
{
    if (_hio_atci.rx_length < 2 || _hio_atci.rx_buffer[0] != 'A' || _hio_atci.rx_buffer[1] != 'T')
    {
        return false;
    }

    if (_hio_atci.rx_length == 2)
    {
        return true;
    }

    _hio_atci.rx_buffer[_hio_atci.rx_length] = 0;

    char *line = _hio_atci.rx_buffer + 2;

    size_t length = _hio_atci.rx_length - 2;

    size_t command_len;

    const hio_atci_command_t *command;

    for (size_t i = 0; i < _hio_atci.commands_length; i++)
    {
        command = _hio_atci.commands + i;

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
                hio_atci_param_t param = {
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

static void _hio_atci_process_character(char character)
{
    if (character == '\n')
    {
        if (!_hio_atci.rx_error && _hio_atci.rx_length > 0)
        {
            bool response = _hio_atci_process_line();

            if (_hio_atci.write_response)
            {
                if (response)
                {
                    hio_atci_write_ok();
                }
                else
                {
                    hio_atci_write_error();
                }
            }
            else
            {
                _hio_atci.write_response = true;
            }
        }
        else if (_hio_atci.rx_error)
        {
            hio_atci_write_error();
        }

        _hio_atci.rx_length = 0;
        _hio_atci.rx_error = false;
    }
    else if (character == '\r')
    {
        return;
    }
    else if (character == '\x1b')
    {
        _hio_atci.rx_length = 0;
        _hio_atci.rx_error = false;
    }
    else if (_hio_atci.rx_length == sizeof(_hio_atci.rx_buffer) - 1)
    {
        _hio_atci.rx_error = true;
    }
    else if (!_hio_atci.rx_error)
    {
        _hio_atci.rx_buffer[_hio_atci.rx_length++] = character;
    }
}

static void _hio_atci_uart_event_handler(hio_uart_channel_t channel, hio_uart_event_t event, void  *event_param)
{
    (void) channel;
    (void) event_param;

    if (event == HIO_UART_EVENT_ASYNC_READ_DATA)
    {
        while (true)
        {
            static uint8_t buffer[16];

            size_t length = hio_uart_async_read(HIO_ATCI_UART, buffer, sizeof(buffer));

            if (length == 0)
            {
                break;
            }

            for (size_t i = 0; i < length; i++)
            {
                _hio_atci_process_character((char) buffer[i]);
            }
        }
    }
}

bool hio_atci_get_uint(hio_atci_param_t *param, uint32_t *value)
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

bool hio_atci_get_string(hio_atci_param_t *param, char *str, size_t length)
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

bool hio_atci_get_buffer_from_hex_string(hio_atci_param_t *param, void *buffer, size_t *length)
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

bool hio_atci_is_comma(hio_atci_param_t *param)
{
    return param->txt[param->offset++] == ',';
}

bool hio_atci_is_quotation_mark(hio_atci_param_t *param)
{
    return param->txt[param->offset++] == '"';
}

void hio_atci_set_uart_active_callback(bool(*callback)(void), hio_tick_t scan_interval)
{
    _hio_atci.uart_active_callback = callback;
    _hio_atci.scan_interval = scan_interval;

    if (callback == NULL)
    {
        if (_hio_atci.vbus_sense_test_task_id)
        {
            hio_scheduler_unregister(_hio_atci.vbus_sense_test_task_id);

            _hio_atci.vbus_sense_test_task_id = 0;
        }
    }
    else
    {
        if (_hio_atci.vbus_sense_test_task_id == 0)
        {
            _hio_atci.vbus_sense_test_task_id = hio_scheduler_register(_hio_atci_uart_active_test_task, NULL, scan_interval);
        }
    }

    _hio_atci_uart_active_test();
}

static void _hio_atci_uart_active_test(void)
{
    if ((_hio_atci.uart_active_callback == NULL) || _hio_atci.uart_active_callback())
    {
        if (!_hio_atci.ready)
        {
            hio_uart_init(HIO_ATCI_UART, HIO_UART_BAUDRATE_115200, HIO_UART_SETTING_8N1);

            hio_uart_set_async_fifo(HIO_ATCI_UART, NULL, &_hio_atci.read_fifo);

            hio_uart_set_event_handler(HIO_ATCI_UART, _hio_atci_uart_event_handler, NULL);

            hio_uart_async_read_start(HIO_ATCI_UART, 1000);

            _hio_atci.ready = true;

            hio_uart_write(HIO_ATCI_UART, "\r\n", 2);
        }
    }
    else
    {
        if (_hio_atci.ready)
        {
            _hio_atci.ready = false;

            hio_uart_deinit(HIO_ATCI_UART);
        }
    }
}

static void _hio_atci_uart_active_test_task(void *param)
{
    (void) param;

    _hio_atci_uart_active_test();

    hio_scheduler_plan_current_relative(_hio_atci.scan_interval);
}
