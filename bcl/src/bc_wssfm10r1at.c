#include <bc_wssfm10r1at.h>

#define BC_WSSFM10R1AT_DELAY_RUN 100
#define BC_WSSFM10R1AT_DELAY_INITIALIZATION_RESET_H 100
#define BC_WSSFM10R1AT_DELAY_INITIALIZATION_AT_COMMAND 100
#define BC_WSSFM10R1AT_DELAY_INITIALIZATION_AT_RESPONSE 100
#define BC_WSSFM10R1AT_DELAY_SET_POWER_RESPONSE 100
#define BC_WSSFM10R1AT_DELAY_SEND_RF_FRAME_RESPONSE 8000
#define BC_WSSFM10R1AT_DELAY_READ_ID_RESPONSE 100
#define BC_WSSFM10R1AT_DELAY_READ_PAC_RESPONSE 100
#define BC_WSSFM10R1AT_DELAY_CONTINUOUS_WAVE_RESPONSE 2000
#define BC_WSSFM10R1AT_DELAY_DEEP_SLEEP_RESPONSE 100

static void _bc_wssfm10r1at_task(void *param);

static void _bc_wssfm10r1at_set_state(bc_wssfm10r1at_t *self, bc_wssfm10r1at_state_t state);

static bool _bc_wssfm10r1at_read_response(bc_wssfm10r1at_t *self);

void bc_wssfm10r1at_init(bc_wssfm10r1at_t *self, bc_gpio_channel_t reset_signal, bc_uart_channel_t uart_channel)
{
    memset(self, 0, sizeof(*self));

    self->_reset_signal = reset_signal;
    self->_uart_channel = uart_channel;

    bc_gpio_init(self->_reset_signal);
    bc_gpio_set_output(self->_reset_signal, true);
    bc_gpio_set_mode(self->_reset_signal, BC_GPIO_MODE_OUTPUT);

    bc_fifo_init(&self->_tx_fifo, self->_tx_fifo_buffer, sizeof(self->_tx_fifo_buffer));
    bc_fifo_init(&self->_rx_fifo, self->_rx_fifo_buffer, sizeof(self->_rx_fifo_buffer));

    bc_uart_param_t uart_param =
    {
        .baudrate = BC_UART_BAUDRATE_9600_BD
    };

    bc_uart_init(self->_uart_channel, &uart_param, &self->_tx_fifo, &self->_rx_fifo);

    self->_task_id = bc_scheduler_register(_bc_wssfm10r1at_task, self, BC_WSSFM10R1AT_DELAY_RUN);

    self->_state = BC_WSSFM10R1AT_STATE_INITIALIZE;
}

void bc_wssfm10r1at_set_event_handler(bc_wssfm10r1at_t *self, void (*event_handler)(bc_wssfm10r1at_t *, bc_wssfm10r1at_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

bool bc_wssfm10r1at_is_ready(bc_wssfm10r1at_t *self)
{
    if (self->_state == BC_WSSFM10R1AT_STATE_READY)
    {
        return true;
    }

    if (self->_state == BC_WSSFM10R1AT_STATE_DEEP_SLEEP)
    {
        return true;
    }

    return false;
}

bool bc_wssfm10r1at_send_rf_frame(bc_wssfm10r1at_t *self, const void *buffer, size_t length)
{
    if (!bc_wssfm10r1at_is_ready(self) || length == 0 || length > 12)
    {
        return false;
    }

    self->_message_length = length;

    memcpy(self->_message_buffer, buffer, self->_message_length);

    _bc_wssfm10r1at_set_state(self, BC_WSSFM10R1AT_STATE_SEND_RF_FRAME_COMMAND);

    return true;
}

bool bc_wssfm10r1at_read_device_id(bc_wssfm10r1at_t *self)
{
    if (!bc_wssfm10r1at_is_ready(self))
    {
        return false;
    }

    _bc_wssfm10r1at_set_state(self, BC_WSSFM10R1AT_STATE_READ_DEVICE_ID_COMMAND);

    return true;
}

bool bc_wssfm10r1at_read_device_pac(bc_wssfm10r1at_t *self)
{
    if (!bc_wssfm10r1at_is_ready(self))
    {
        return false;
    }

    _bc_wssfm10r1at_set_state(self, BC_WSSFM10R1AT_STATE_READ_DEVICE_PAC_COMMAND);

    return true;
}

bool bc_wssfm10r1at_continuous_wave(bc_wssfm10r1at_t *self)
{
    if (!bc_wssfm10r1at_is_ready(self))
    {
        return false;
    }

    _bc_wssfm10r1at_set_state(self, BC_WSSFM10R1AT_STATE_CONTINUOUS_WAVE_COMMAND);

    return true;
}

static void _bc_wssfm10r1at_task(void *param)
{
    bc_wssfm10r1at_t *self = param;

    while (true)
    {
        switch (self->_state)
        {
            case BC_WSSFM10R1AT_STATE_READY:
            {
                if (self->_deep_sleep)
                {
                    self->_deep_sleep = false;

                    self->_state = self->_state_after_sleep;

                    continue;
                }

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_WSSFM10R1AT_EVENT_READY, self->_event_param);
                }

                if (self->_state == BC_WSSFM10R1AT_STATE_READY)
                {
                    self->_state = BC_WSSFM10R1AT_STATE_DEEP_SLEEP_COMMAND;
                }

                continue;
            }
            case BC_WSSFM10R1AT_STATE_ERROR:
            {
                self->_deep_sleep = false;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_WSSFM10R1AT_EVENT_ERROR, self->_event_param);
                }

                self->_state = BC_WSSFM10R1AT_STATE_INITIALIZE;

                continue;
            }
            case BC_WSSFM10R1AT_STATE_INITIALIZE:
            {
                self->_state = BC_WSSFM10R1AT_STATE_INITIALIZE_RESET_L;

                continue;
            }
            case BC_WSSFM10R1AT_STATE_INITIALIZE_RESET_L:
            {
                bc_gpio_set_output(self->_reset_signal, false);

                self->_state = BC_WSSFM10R1AT_STATE_INITIALIZE_RESET_H;

                bc_scheduler_plan_current_relative(BC_WSSFM10R1AT_DELAY_INITIALIZATION_RESET_H);

                return;
            }
            case BC_WSSFM10R1AT_STATE_INITIALIZE_RESET_H:
            {
                bc_gpio_set_output(self->_reset_signal, true);

                self->_state = BC_WSSFM10R1AT_STATE_INITIALIZE_AT_COMMAND;

                bc_scheduler_plan_current_relative(BC_WSSFM10R1AT_DELAY_INITIALIZATION_AT_COMMAND);

                return;
            }
            case BC_WSSFM10R1AT_STATE_INITIALIZE_AT_COMMAND:
            {
                self->_state = BC_WSSFM10R1AT_STATE_ERROR;

                // TODO Purge RX FIFO

                strcpy(self->_command, "\rAT\r");

                size_t length = strlen(self->_command);

                if (bc_uart_write(self->_uart_channel, self->_command, length, 0) != length)
                {
                    continue;
                }

                bc_gpio_set_output(self->_reset_signal, true);

                self->_state = BC_WSSFM10R1AT_STATE_INITIALIZE_AT_RESPONSE;

                bc_scheduler_plan_current_relative(BC_WSSFM10R1AT_DELAY_INITIALIZATION_AT_RESPONSE);

                return;
            }
            case BC_WSSFM10R1AT_STATE_INITIALIZE_AT_RESPONSE:
            {
                self->_state = BC_WSSFM10R1AT_STATE_ERROR;

                if (!_bc_wssfm10r1at_read_response(self))
                {
                    continue;
                }

                if (strcmp(self->_response, "OK\r") != 0)
                {
                    continue;
                }

                self->_state = BC_WSSFM10R1AT_STATE_SET_POWER_COMMAND;

                continue;
            }
            case BC_WSSFM10R1AT_STATE_SET_POWER_COMMAND:
            {
                self->_state = BC_WSSFM10R1AT_STATE_ERROR;

                strcpy(self->_command, "ATS302=15\r");

                size_t length = strlen(self->_command);

                if (bc_uart_write(self->_uart_channel, self->_command, length, 0) != length)
                {
                    continue;
                }

                self->_state = BC_WSSFM10R1AT_STATE_SET_POWER_RESPONSE;

                bc_scheduler_plan_current_relative(BC_WSSFM10R1AT_DELAY_SET_POWER_RESPONSE);

                return;
            }
            case BC_WSSFM10R1AT_STATE_SET_POWER_RESPONSE:
            {
                self->_state = BC_WSSFM10R1AT_STATE_ERROR;

                if (!_bc_wssfm10r1at_read_response(self))
                {
                    continue;
                }

                if (strcmp(self->_response, "OK\r") != 0)
                {
                    continue;
                }

                self->_state = BC_WSSFM10R1AT_STATE_READY;

                continue;
            }
            case BC_WSSFM10R1AT_STATE_SEND_RF_FRAME_COMMAND:
            {
                self->_state = BC_WSSFM10R1AT_STATE_ERROR;

                static const char *hex_lookup_table[] =
                {
                    "00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0A", "0B", "0C", "0D", "0E", "0F",
                    "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1A", "1B", "1C", "1D", "1E", "1F",
                    "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2A", "2B", "2C", "2D", "2E", "2F",
                    "30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3A", "3B", "3C", "3D", "3E", "3F",
                    "40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4A", "4B", "4C", "4D", "4E", "4F",
                    "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
                    "60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6A", "6B", "6C", "6D", "6E", "6F",
                    "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
                    "80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
                    "90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9A", "9B", "9C", "9D", "9E", "9F",
                    "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF",
                    "B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
                    "C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF",
                    "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF",
                    "E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
                    "F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF"
                };

                strcpy(self->_command, "AT$SF=");

                for (size_t i = 0; i < self->_message_length; i++)
                {
                    strcat(self->_command, hex_lookup_table[*((uint8_t *) self->_message_buffer + i)]);
                }

                strcat(self->_command, "\r");

                size_t length = strlen(self->_command);

                if (bc_uart_write(self->_uart_channel, self->_command, length, 0) != length)
                {
                    continue;
                }

                self->_state = BC_WSSFM10R1AT_STATE_SEND_RF_FRAME_RESPONSE;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_WSSFM10R1AT_EVENT_SEND_RF_FRAME_START, self->_event_param);
                }

                bc_scheduler_plan_current_relative(BC_WSSFM10R1AT_DELAY_SEND_RF_FRAME_RESPONSE);

                return;
            }
            case BC_WSSFM10R1AT_STATE_SEND_RF_FRAME_RESPONSE:
            {
                self->_state = BC_WSSFM10R1AT_STATE_ERROR;

                if (!_bc_wssfm10r1at_read_response(self))
                {
                    continue;
                }

                if (strcmp(self->_response, "OK\r") != 0)
                {
                    continue;
                }

                self->_state = BC_WSSFM10R1AT_STATE_READY;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_WSSFM10R1AT_EVENT_SEND_RF_FRAME_DONE, self->_event_param);
                }

                continue;
            }
            case BC_WSSFM10R1AT_STATE_READ_DEVICE_ID_COMMAND:
            {
                self->_state = BC_WSSFM10R1AT_STATE_ERROR;

                strcpy(self->_command, "AT$I=10\r");

                size_t length = strlen(self->_command);

                if (bc_uart_write(self->_uart_channel, self->_command, length, 0) != length)
                {
                    continue;
                }

                self->_state = BC_WSSFM10R1AT_STATE_READ_DEVICE_ID_RESPONSE;

                bc_scheduler_plan_current_relative(BC_WSSFM10R1AT_DELAY_READ_PAC_RESPONSE);

                return;
            }
            case BC_WSSFM10R1AT_STATE_READ_DEVICE_ID_RESPONSE:
            {
                self->_state = BC_WSSFM10R1AT_STATE_ERROR;

                if (!_bc_wssfm10r1at_read_response(self))
                {
                    continue;
                }

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_WSSFM10R1AT_EVENT_READ_DEVICE_ID, self->_event_param);
                }

                self->_state = BC_WSSFM10R1AT_STATE_READY;

                continue;
            }
            case BC_WSSFM10R1AT_STATE_READ_DEVICE_PAC_COMMAND:
            {
                self->_state = BC_WSSFM10R1AT_STATE_ERROR;

                strcpy(self->_command, "AT$I=11\r");

                size_t length = strlen(self->_command);

                if (bc_uart_write(self->_uart_channel, self->_command, length, 0) != length)
                {
                    continue;
                }

                self->_state = BC_WSSFM10R1AT_STATE_READ_DEVICE_PAC_RESPONSE;

                bc_scheduler_plan_current_relative(BC_WSSFM10R1AT_DELAY_READ_PAC_RESPONSE);

                return;
            }
            case BC_WSSFM10R1AT_STATE_READ_DEVICE_PAC_RESPONSE:
            {
                self->_state = BC_WSSFM10R1AT_STATE_ERROR;

                if (!_bc_wssfm10r1at_read_response(self))
                {
                    continue;
                }

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_WSSFM10R1AT_EVENT_READ_DEVICE_PAC, self->_event_param);
                }

                self->_state = BC_WSSFM10R1AT_STATE_READY;

                continue;
            }
            case BC_WSSFM10R1AT_STATE_CONTINUOUS_WAVE_COMMAND:
            {
                self->_state = BC_WSSFM10R1AT_STATE_ERROR;

                strcpy(self->_command, "AT$CW=868130000,1,15\r");

                size_t length = strlen(self->_command);

                if (bc_uart_write(self->_uart_channel, self->_command, length, 0) != length)
                {
                    continue;
                }

                self->_state = BC_WSSFM10R1AT_STATE_CONTINUOUS_WAVE_RESPONSE;

                bc_scheduler_plan_current_relative(BC_WSSFM10R1AT_DELAY_CONTINUOUS_WAVE_RESPONSE);

                return;
            }
            case BC_WSSFM10R1AT_STATE_CONTINUOUS_WAVE_RESPONSE:
            {
                self->_state = BC_WSSFM10R1AT_STATE_ERROR;

                if (!_bc_wssfm10r1at_read_response(self))
                {
                    continue;
                }

                if (strcmp(self->_response, "OK\r") != 0)
                {
                    continue;
                }

                self->_state = BC_WSSFM10R1AT_STATE_CONTINUOUS_WAVE;

                continue;
            }
            case BC_WSSFM10R1AT_STATE_CONTINUOUS_WAVE:
            {
                return;
            }
            case BC_WSSFM10R1AT_STATE_DEEP_SLEEP_COMMAND:
            {
                self->_state = BC_WSSFM10R1AT_STATE_ERROR;

                strcpy(self->_command, "AT$P=2\r");

                size_t length = strlen(self->_command);

                if (bc_uart_write(self->_uart_channel, self->_command, length, 0) != length)
                {
                    continue;
                }

                self->_state = BC_WSSFM10R1AT_STATE_DEEP_SLEEP_RESPONSE;

                bc_scheduler_plan_current_relative(BC_WSSFM10R1AT_DELAY_DEEP_SLEEP_RESPONSE);

                return;
            }
            case BC_WSSFM10R1AT_STATE_DEEP_SLEEP_RESPONSE:
            {
                self->_state = BC_WSSFM10R1AT_STATE_ERROR;

                if (!_bc_wssfm10r1at_read_response(self))
                {
                    continue;
                }

                if (strcmp(self->_response, "OK\r") != 0)
                {
                    continue;
                }

                self->_state = BC_WSSFM10R1AT_STATE_DEEP_SLEEP;

                continue;
            }
            case BC_WSSFM10R1AT_STATE_DEEP_SLEEP:
            {
                self->_deep_sleep = true;

                return;
            }
            default:
            {
                break;
            }
        }
    }
}

static void _bc_wssfm10r1at_set_state(bc_wssfm10r1at_t *self, bc_wssfm10r1at_state_t state)
{
    if (self->_deep_sleep)
    {
        self->_state = BC_WSSFM10R1AT_STATE_INITIALIZE;

        self->_state_after_sleep = state;
    }
    else
    {
        self->_state = state;
    }

    bc_scheduler_plan_now(self->_task_id);
}

static bool _bc_wssfm10r1at_read_response(bc_wssfm10r1at_t *self)
{
    size_t length = 0;

    while (true)
    {
        char rx_character;

        if (bc_uart_read(self->_uart_channel, &rx_character, 1, 0) == 0)
        {
            return false;
        }

        if (rx_character == '\n')
        {
            continue;
        }

        self->_response[length++] = rx_character;

        if (rx_character == '\r')
        {
            if (length == 1)
            {
                length = 0;

                continue;
            }

            self->_response[length] = '\0';

            break;
        }

        if (length == sizeof(self->_response) - 1)
        {
            return false;
        }
    }

    return true;
}
