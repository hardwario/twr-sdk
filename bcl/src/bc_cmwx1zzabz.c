#include <bc_cmwx1zzabz.h>

#define BC_CMWX1ZZABZ_DELAY_RUN 100
#define BC_CMWX1ZZABZ_DELAY_INITIALIZATION_RESET_H 100
#define BC_CMWX1ZZABZ_DELAY_INITIALIZATION_AT_COMMAND 100
#define BC_CMWX1ZZABZ_DELAY_INITIALIZATION_AT_RESPONSE 100
#define BC_CMWX1ZZABZ_DELAY_SET_POWER_RESPONSE 100
#define BC_CMWX1ZZABZ_DELAY_SEND_RF_FRAME_RESPONSE 12000
#define BC_CMWX1ZZABZ_DELAY_READ_ID_RESPONSE 100
#define BC_CMWX1ZZABZ_DELAY_READ_PAC_RESPONSE 100
#define BC_CMWX1ZZABZ_DELAY_CONTINUOUS_WAVE_RESPONSE 2000
#define BC_CMWX1ZZABZ_DELAY_DEEP_SLEEP_RESPONSE 100

// Apply changes to the factory configuration
const char *_init_commands[] = {  
                            "AT\r",
                            "AT+DUTYCYCLE=0\r",
                             NULL
                        };

static void _bc_cmwx1zzabz_task(void *param);

static void _bc_cmwx1zzabz_set_state(bc_cmwx1zzabz_t *self, bc_cmwx1zzabz_state_t state);

static bool _bc_cmwx1zzabz_read_response(bc_cmwx1zzabz_t *self);

void bc_cmwx1zzabz_init(bc_cmwx1zzabz_t *self,  bc_uart_channel_t uart_channel)
{
    memset(self, 0, sizeof(*self));

    self->_uart_channel = uart_channel;

    bc_fifo_init(&self->_tx_fifo, self->_tx_fifo_buffer, sizeof(self->_tx_fifo_buffer));
    bc_fifo_init(&self->_rx_fifo, self->_rx_fifo_buffer, sizeof(self->_rx_fifo_buffer));

    bc_uart_init(self->_uart_channel, BC_UART_BAUDRATE_19200, BC_UART_SETTING_8N1);
    bc_uart_set_async_fifo(self->_uart_channel, &self->_tx_fifo, &self->_rx_fifo);
    bc_uart_async_read_start(self->_uart_channel, BC_TICK_INFINITY);

    self->_task_id = bc_scheduler_register(_bc_cmwx1zzabz_task, self, BC_CMWX1ZZABZ_DELAY_RUN);

    self->_state = BC_CMWX1ZZABZ_STATE_INITIALIZE;
}

void bc_cmwx1zzabz_set_event_handler(bc_cmwx1zzabz_t *self, void (*event_handler)(bc_cmwx1zzabz_t *, bc_cmwx1zzabz_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

bool bc_cmwx1zzabz_is_ready(bc_cmwx1zzabz_t *self)
{
    if (self->_state == BC_CMWX1ZZABZ_STATE_IDLE)
    {
        return true;
    }

    return false;
}

bool bc_cmwx1zzabz_send_rf_frame(bc_cmwx1zzabz_t *self, const void *buffer, size_t length)
{
    if (!bc_cmwx1zzabz_is_ready(self) || length == 0 || length > 12)
    {
        return false;
    }

    self->_message_length = length;

    memcpy(self->_message_buffer, buffer, self->_message_length);

    _bc_cmwx1zzabz_set_state(self, BC_CMWX1ZZABZ_STATE_SEND_RF_FRAME_COMMAND);

    return true;
}


static void _bc_cmwx1zzabz_task(void *param)
{
    bc_cmwx1zzabz_t *self = param;

    while (true)
    {
        switch (self->_state)
        {
            case BC_CMWX1ZZABZ_STATE_READY:
            {
                if (self->_deep_sleep)
                {
                    self->_deep_sleep = false;

                    self->_state = self->_state_after_sleep;

                    continue;
                }

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_CMWX1ZZABZ_EVENT_READY, self->_event_param);
                }

                self->_state = BC_CMWX1ZZABZ_STATE_IDLE;

                continue;
            }
            case BC_CMWX1ZZABZ_STATE_IDLE:
            { 
                // idle
                return;
            }
            case BC_CMWX1ZZABZ_STATE_ERROR:
            {
                self->_deep_sleep = false;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_CMWX1ZZABZ_EVENT_ERROR, self->_event_param);
                }

                self->_state = BC_CMWX1ZZABZ_STATE_INITIALIZE;

                continue;
            }
            case BC_CMWX1ZZABZ_STATE_INITIALIZE:
            {
                self->init_command_index = 0;
                self->_state = BC_CMWX1ZZABZ_STATE_INITIALIZE_COMMAND_SEND;

                continue;
            }

            case BC_CMWX1ZZABZ_STATE_INITIALIZE_COMMAND_SEND:
            {
                self->_state = BC_CMWX1ZZABZ_STATE_ERROR;

                // Purge RX FIFO
                char rx_character;
                while(bc_uart_async_read(self->_uart_channel, &rx_character, 1) != 0)
                {
                }

                strcpy(self->_command, _init_commands[self->init_command_index]);
                size_t length = strlen(self->_command);

                if (bc_uart_async_write(self->_uart_channel, self->_command, length) != length)
                {
                    continue;
                }

                self->_state = BC_CMWX1ZZABZ_STATE_INITIALIZE_COMMAND_RESPONSE;
                bc_scheduler_plan_current_from_now(BC_CMWX1ZZABZ_DELAY_INITIALIZATION_AT_COMMAND);

                return;
            }
            case BC_CMWX1ZZABZ_STATE_INITIALIZE_COMMAND_RESPONSE:
            {
                self->_state = BC_CMWX1ZZABZ_STATE_ERROR;

                if (!_bc_cmwx1zzabz_read_response(self))
                {
                    volatile int a = 5;
                    a++;
                    continue;
                }

                if (strcmp(self->_response, "+OK\r") != 0)
                {
                    volatile int b = 7;
                    b++;
                    continue;
                }

                self->init_command_index++;

                if(_init_commands[self->init_command_index] == NULL)
                {
                    self->_state = BC_CMWX1ZZABZ_STATE_READY;
                }
                else
                {
                    self->_state = BC_CMWX1ZZABZ_STATE_INITIALIZE_COMMAND_SEND;
                }

                continue;
            }
            case BC_CMWX1ZZABZ_STATE_SEND_RF_FRAME_COMMAND:
            {
                self->_state = BC_CMWX1ZZABZ_STATE_ERROR;

                snprintf(self->_command, BC_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+UTX %d\r", self->_message_length);

                uint8_t command_length = strlen(self->_command);

                for (size_t i = 0; i < self->_message_length; i++)
                {
                    //strcat(self->_command, hex_lookup_table[*((uint8_t *) self->_message_buffer + i)]);
                    // we put directly binary data to the buffer
                    self->_command[command_length + i] = self->_message_buffer[i];
                }

                self->_command[command_length + self->_message_length] = '\r';

                size_t length = command_length + self->_message_length + 1; // 1 for \n

                if (bc_uart_async_write(self->_uart_channel, self->_command, length) != length)
                {
                    continue;
                }

                self->_state = BC_CMWX1ZZABZ_STATE_SEND_RF_FRAME_RESPONSE;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_CMWX1ZZABZ_EVENT_SEND_RF_FRAME_START, self->_event_param);
                }

                bc_scheduler_plan_current_from_now(BC_CMWX1ZZABZ_DELAY_SEND_RF_FRAME_RESPONSE);

                return;
            }
            case BC_CMWX1ZZABZ_STATE_SEND_RF_FRAME_RESPONSE:
            {
                self->_state = BC_CMWX1ZZABZ_STATE_ERROR;

                if (!_bc_cmwx1zzabz_read_response(self))
                {
                    continue;
                }

                if (strcmp(self->_response, "+OK\r") != 0)
                {
                    continue;
                }

                self->_state = BC_CMWX1ZZABZ_STATE_READY;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_CMWX1ZZABZ_EVENT_SEND_RF_FRAME_DONE, self->_event_param);
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

static void _bc_cmwx1zzabz_set_state(bc_cmwx1zzabz_t *self, bc_cmwx1zzabz_state_t state)
{
    if (self->_deep_sleep)
    {
        self->_state = BC_CMWX1ZZABZ_STATE_INITIALIZE;

        self->_state_after_sleep = state;
    }
    else
    {
        self->_state = state;
    }

    bc_scheduler_plan_now(self->_task_id);
}

static bool _bc_cmwx1zzabz_read_response(bc_cmwx1zzabz_t *self)
{
    size_t length = 0;

    while (true)
    {
        char rx_character;

        if (bc_uart_async_read(self->_uart_channel, &rx_character, 1) == 0)
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
