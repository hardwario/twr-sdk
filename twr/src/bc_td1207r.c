#include <twr_td1207r.h>

#define TWR_TD1207R_DELAY_RUN 100
#define TWR_TD1207R_DELAY_INITIALIZATION_RESET_H 100
#define TWR_TD1207R_DELAY_INITIALIZATION_AT_COMMAND 3000
#define TWR_TD1207R_DELAY_INITIALIZATION_AT_RESPONSE 100
#define TWR_TD1207R_DELAY_SEND_RF_FRAME_RESPONSE 8000

static void _twr_td1207r_task(void *param);

static bool _twr_td1207r_read_response(twr_td1207r_t *self);

void twr_td1207r_init(twr_td1207r_t *self, twr_gpio_channel_t reset_signal, twr_uart_channel_t uart_channel)
{
    memset(self, 0, sizeof(*self));

    self->_reset_signal = reset_signal;
    self->_uart_channel = uart_channel;

    twr_gpio_init(self->_reset_signal);
    twr_gpio_set_output(self->_reset_signal, 1);
    twr_gpio_set_mode(self->_reset_signal, TWR_GPIO_MODE_OUTPUT);

    twr_fifo_init(&self->_tx_fifo, self->_tx_fifo_buffer, sizeof(self->_tx_fifo_buffer));
    twr_fifo_init(&self->_rx_fifo, self->_rx_fifo_buffer, sizeof(self->_rx_fifo_buffer));

    twr_uart_init(self->_uart_channel, TWR_UART_BAUDRATE_9600, TWR_UART_SETTING_8N1);
    twr_uart_set_async_fifo(self->_uart_channel, &self->_tx_fifo, &self->_rx_fifo);
    twr_uart_async_read_start(self->_uart_channel, TWR_TICK_INFINITY);

    self->_task_id = twr_scheduler_register(_twr_td1207r_task, self, TWR_TD1207R_DELAY_RUN);

    self->_state = TWR_TD1207R_STATE_INITIALIZE;
}

void twr_td1207r_set_event_handler(twr_td1207r_t *self, void (*event_handler)(twr_td1207r_t *, twr_td1207r_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

bool twr_td1207r_is_ready(twr_td1207r_t *self)
{
    return self->_state == TWR_TD1207R_STATE_READY ? true : false;
}

bool twr_td1207r_send_rf_frame(twr_td1207r_t *self, const void *buffer, size_t length)
{
    if (!twr_td1207r_is_ready(self) || length == 0 || length > 12)
    {
        return false;
    }

    self->_message_length = length;

    memcpy(self->_message_buffer, buffer, self->_message_length);

    self->_state = TWR_TD1207R_STATE_SEND_RF_FRAME_COMMAND;

    twr_scheduler_plan_now(self->_task_id);

    return true;
}

static void _twr_td1207r_task(void *param)
{
    twr_td1207r_t *self = param;

    while (true)
    {
        switch (self->_state)
        {
            case TWR_TD1207R_STATE_READY:
            {
                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, TWR_TD1207R_EVENT_READY, self->_event_param);
                }

                return;
            }
            case TWR_TD1207R_STATE_ERROR:
            {
                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, TWR_TD1207R_EVENT_ERROR, self->_event_param);
                }

                self->_state = TWR_TD1207R_STATE_INITIALIZE;

                continue;
            }
            case TWR_TD1207R_STATE_INITIALIZE:
            {
                self->_state = TWR_TD1207R_STATE_INITIALIZE_RESET_L;

                continue;
            }
            case TWR_TD1207R_STATE_INITIALIZE_RESET_L:
            {
                twr_gpio_set_output(self->_reset_signal, 0);

                self->_state = TWR_TD1207R_STATE_INITIALIZE_RESET_H;

                twr_scheduler_plan_current_from_now(TWR_TD1207R_DELAY_INITIALIZATION_RESET_H);

                return;
            }
            case TWR_TD1207R_STATE_INITIALIZE_RESET_H:
            {
                twr_gpio_set_output(self->_reset_signal, 1);

                self->_state = TWR_TD1207R_STATE_INITIALIZE_AT_COMMAND;

                twr_scheduler_plan_current_from_now(TWR_TD1207R_DELAY_INITIALIZATION_AT_COMMAND);

                return;
            }
            case TWR_TD1207R_STATE_INITIALIZE_AT_COMMAND:
            {
                self->_state = TWR_TD1207R_STATE_ERROR;

                // TODO Purge RX FIFO

                strcpy(self->_command, "\rAT\r");

                size_t length = strlen(self->_command);

                if (twr_uart_async_write(self->_uart_channel, self->_command, length) != length)
                {
                    continue;
                }

                self->_state = TWR_TD1207R_STATE_INITIALIZE_AT_RESPONSE;

                twr_scheduler_plan_current_from_now(TWR_TD1207R_DELAY_INITIALIZATION_AT_RESPONSE);

                return;
            }
            case TWR_TD1207R_STATE_INITIALIZE_AT_RESPONSE:
            {
                self->_state = TWR_TD1207R_STATE_ERROR;

                if (!_twr_td1207r_read_response(self))
                {
                    continue;
                }

                if (strcmp(self->_response, self->_command + 1) != 0)
                {
                    continue;
                }

                if (!_twr_td1207r_read_response(self))
                {
                    continue;
                }

                if (strcmp(self->_response, "OK\r") != 0)
                {
                    continue;
                }

                self->_state = TWR_TD1207R_STATE_READY;

                continue;
            }
            case TWR_TD1207R_STATE_SEND_RF_FRAME_COMMAND:
            {
                self->_state = TWR_TD1207R_STATE_ERROR;

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

                if (twr_uart_async_write(self->_uart_channel, self->_command, length) != length)
                {
                    continue;
                }

                self->_state = TWR_TD1207R_STATE_SEND_RF_FRAME_RESPONSE;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, TWR_TD1207R_EVENT_SEND_RF_FRAME_START, self->_event_param);
                }

                twr_scheduler_plan_current_from_now(TWR_TD1207R_DELAY_SEND_RF_FRAME_RESPONSE);

                return;
            }
            case TWR_TD1207R_STATE_SEND_RF_FRAME_RESPONSE:
            {
                self->_state = TWR_TD1207R_STATE_ERROR;

                if (!_twr_td1207r_read_response(self))
                {
                    continue;
                }

                if (strcmp(self->_response, self->_command) != 0)
                {
                    continue;
                }

                if (!_twr_td1207r_read_response(self))
                {
                    continue;
                }

                if (strcmp(self->_response, "OK\r") != 0)
                {
                    continue;
                }

                self->_state = TWR_TD1207R_STATE_READY;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, TWR_TD1207R_EVENT_SEND_RF_FRAME_DONE, self->_event_param);
                }

                continue;
            }
            default:
            {
                break;
            }
        }
    }
}

static bool _twr_td1207r_read_response(twr_td1207r_t *self)
{
    size_t length = 0;

    while (true)
    {
        char rx_character;

        if (twr_uart_async_read(self->_uart_channel, &rx_character, 1) == 0)
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
