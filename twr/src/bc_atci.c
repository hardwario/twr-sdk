#include <bc_atci.h>
#include <bc_scheduler.h>
#include <bc_system.h>

static void _bc_atci_uart_event_handler(bc_uart_channel_t channel, bc_uart_event_t event, void  *event_param);
static void _bc_atci_uart_active_test(void);
static void _bc_atci_uart_active_test_task(void  *param);

static struct
{
    const bc_atci_command_t *commands;
    size_t commands_length;
    char tx_buffer[256];
    char rx_buffer[256];
    size_t rx_length;
    bool rx_error;
    uint8_t read_fifo_buffer[128];
    bc_fifo_t read_fifo;
    bc_scheduler_task_id_t vbus_sense_test_task_id;
    bool ready;
    bool (*uart_active_callback)(void);
    bc_tick_t scan_interval;
    bool write_response;

} _bc_atci;

void bc_atci_init(const bc_atci_command_t *commands, int length)
{
    memset(&_bc_atci, 0, sizeof(_bc_atci));

    _bc_atci.commands = commands;

    _bc_atci.commands_length = length;

    _bc_atci.rx_length = 0;

    _bc_atci.rx_error = false;

    _bc_atci.write_response = true;

    bc_fifo_init(&_bc_atci.read_fifo, _bc_atci.read_fifo_buffer, sizeof(_bc_atci.read_fifo_buffer));

    bc_atci_set_uart_active_callback(bc_system_get_vbus_sense, 200);
}

void bc_atci_printf(const char *format, ...)
{
    if (!_bc_atci.ready)
    {
        return;
    }

    va_list ap;

    size_t length;

    va_start(ap, format);

    length = vsnprintf(_bc_atci.tx_buffer, sizeof(_bc_atci.tx_buffer) - 2, format, ap);

    va_end(ap);

    _bc_atci.tx_buffer[length++] = '\r';
    _bc_atci.tx_buffer[length++] = '\n';

    bc_uart_write(BC_ATCI_UART, _bc_atci.tx_buffer, length);
}

bool bc_atci_skip_response(void)
{
    _bc_atci.write_response = false;

    return true;
}

void bc_atci_write_ok(void)
{
    bc_uart_write(BC_ATCI_UART, "OK\r\n", 4);
}

void bc_atci_write_error(void)
{
    bc_uart_write(BC_ATCI_UART, "ERROR\r\n", 7);
}

bool bc_atci_clac_action(void)
{
    for (size_t i = 0; i < _bc_atci.commands_length; i++)
    {
        bc_atci_printf("AT%s", _bc_atci.commands[i].command);
    }
    return true;
}

bool bc_atci_help_action(void)
{
    for (size_t i = 0; i < _bc_atci.commands_length; i++)
    {
        bc_atci_printf("AT%s %s", _bc_atci.commands[i].command, _bc_atci.commands[i].hint);
    }
    return true;
}

static bool _bc_atci_process_line(void)
{
    if (_bc_atci.rx_length < 2 || _bc_atci.rx_buffer[0] != 'A' || _bc_atci.rx_buffer[1] != 'T')
    {
        return false;
    }

    if (_bc_atci.rx_length == 2)
    {
        return true;
    }

    _bc_atci.rx_buffer[_bc_atci.rx_length] = 0;

    char *line = _bc_atci.rx_buffer + 2;

    size_t length = _bc_atci.rx_length - 2;

    size_t command_len;

    const bc_atci_command_t *command;

    for (size_t i = 0; i < _bc_atci.commands_length; i++)
    {
        command = _bc_atci.commands + i;

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
                bc_atci_param_t param = {
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

static void _bc_atci_process_character(char character)
{
    if (character == '\n')
    {
        if (!_bc_atci.rx_error && _bc_atci.rx_length > 0)
        {
            bool response = _bc_atci_process_line();

            if (_bc_atci.write_response)
            {
                if (response)
                {
                    bc_atci_write_ok();
                }
                else
                {
                    bc_atci_write_error();
                }
            }
            else
            {
                _bc_atci.write_response = true;
            }
        }
        else if (_bc_atci.rx_error)
        {
            bc_atci_write_error();
        }

        _bc_atci.rx_length = 0;
        _bc_atci.rx_error = false;
    }
    else if (character == '\r')
    {
        return;
    }
    else if (character == '\x1b')
    {
        _bc_atci.rx_length = 0;
        _bc_atci.rx_error = false;
    }
    else if (_bc_atci.rx_length == sizeof(_bc_atci.rx_buffer) - 1)
    {
        _bc_atci.rx_error = true;
    }
    else if (!_bc_atci.rx_error)
    {
        _bc_atci.rx_buffer[_bc_atci.rx_length++] = character;
    }
}

static void _bc_atci_uart_event_handler(bc_uart_channel_t channel, bc_uart_event_t event, void  *event_param)
{
    (void) channel;
    (void) event_param;

    if (event == BC_UART_EVENT_ASYNC_READ_DATA)
    {
        while (true)
        {
            static uint8_t buffer[16];

            size_t length = bc_uart_async_read(BC_ATCI_UART, buffer, sizeof(buffer));

            if (length == 0)
            {
                break;
            }

            for (size_t i = 0; i < length; i++)
            {
                _bc_atci_process_character((char) buffer[i]);
            }
        }
    }
}

bool bc_atci_get_uint(bc_atci_param_t *param, uint32_t *value)
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

bool bc_atci_get_string(bc_atci_param_t *param, char *str, size_t length)
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

bool bc_atci_get_buffer_from_hex_string(bc_atci_param_t *param, void *buffer, size_t *length)
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

bool bc_atci_is_comma(bc_atci_param_t *param)
{
    return param->txt[param->offset++] == ',';
}

bool bc_atci_is_quotation_mark(bc_atci_param_t *param)
{
    return param->txt[param->offset++] == '"';
}

void bc_atci_set_uart_active_callback(bool(*callback)(void), bc_tick_t scan_interval)
{
    _bc_atci.uart_active_callback = callback;
    _bc_atci.scan_interval = scan_interval;

    if (callback == NULL)
    {
        if (_bc_atci.vbus_sense_test_task_id)
        {
            bc_scheduler_unregister(_bc_atci.vbus_sense_test_task_id);

            _bc_atci.vbus_sense_test_task_id = 0;
        }
    }
    else
    {
        if (_bc_atci.vbus_sense_test_task_id == 0)
        {
            _bc_atci.vbus_sense_test_task_id = bc_scheduler_register(_bc_atci_uart_active_test_task, NULL, scan_interval);
        }
    }

    _bc_atci_uart_active_test();
}

static void _bc_atci_uart_active_test(void)
{
    if ((_bc_atci.uart_active_callback == NULL) || _bc_atci.uart_active_callback())
    {
        if (!_bc_atci.ready)
        {
            bc_uart_init(BC_ATCI_UART, BC_UART_BAUDRATE_115200, BC_UART_SETTING_8N1);

            bc_uart_set_async_fifo(BC_ATCI_UART, NULL, &_bc_atci.read_fifo);

            bc_uart_set_event_handler(BC_ATCI_UART, _bc_atci_uart_event_handler, NULL);

            bc_uart_async_read_start(BC_ATCI_UART, 1000);

            _bc_atci.ready = true;

            bc_uart_write(BC_ATCI_UART, "\r\n", 2);
        }
    }
    else
    {
        if (_bc_atci.ready)
        {
            _bc_atci.ready = false;

            bc_uart_deinit(BC_ATCI_UART);
        }
    }
}

static void _bc_atci_uart_active_test_task(void *param)
{
    (void) param;

    _bc_atci_uart_active_test();

    bc_scheduler_plan_current_relative(_bc_atci.scan_interval);
}
