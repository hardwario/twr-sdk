#include <bc_atci.h>
#include <bc_scheduler.h>
#include <bc_system.h>

#define _BC_ATCI_UART_VBUS_SCAN_TIME     200

static void _bc_atci_uart_event_handler(bc_uart_channel_t channel, bc_uart_event_t event, void  *event_param);
static void _bc_atci_uart_vbus_sense_test_task(void  *param);

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
    bc_tick_t vbus_sense_test_task_id;
    bool ready;

} _bc_atci;

void bc_atci_init(const bc_atci_command_t *commands, int length)
{
    _bc_atci.commands = commands;

    _bc_atci.commands_length = length;

    _bc_atci.rx_length = 0;

    _bc_atci.rx_error = false;

    bc_fifo_init(&_bc_atci.read_fifo, _bc_atci.read_fifo_buffer, sizeof(_bc_atci.read_fifo_buffer));

    _bc_atci.vbus_sense_test_task_id = bc_scheduler_register(_bc_atci_uart_vbus_sense_test_task, NULL, 0);
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
            if (_bc_atci_process_line())
            {
                bc_atci_write_ok();
            }
            else
            {
                bc_atci_write_error();
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

static void _bc_atci_uart_vbus_sense_test_task(void *param)
{
    (void) param;

    if (bc_system_get_vbus_sense())
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

    bc_scheduler_plan_current_relative(_BC_ATCI_UART_VBUS_SCAN_TIME);
}
