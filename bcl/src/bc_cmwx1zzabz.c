#include <bc_cmwx1zzabz.h>

#define BC_CMWX1ZZABZ_DELAY_RUN 100
#define BC_CMWX1ZZABZ_DELAY_INITIALIZATION_RESET_H 100
#define BC_CMWX1ZZABZ_DELAY_INITIALIZATION_AT_COMMAND 100 // ! when using longer AT responses
#define BC_CMWX1ZZABZ_DELAY_INITIALIZATION_AT_RESPONSE 100
#define BC_CMWX1ZZABZ_DELAY_SEND_RF_FRAME_RESPONSE 3000
#define BC_CMWX1ZZABZ_DELAY_JOIN_RESPONSE 8000

// Apply changes to the factory configuration
const char *_init_commands[] = {  
                           /* "AT+UART=9600\r"*/
                            "AT+SLEEP?\r",
                            "AT\r",
                            "AT+DUTYCYCLE=0\r",
                            "AT+DEVADDR?\r",
                            "AT+DEVEUI?\r",
                            "AT+APPEUI?\r",
                            "AT+NWKSKEY?\r",
                            "AT+APPSKEY?\r",
                            "AT+APPKEY?\r",
                            "AT+BAND?\r",
                            "AT+MODE?\r",
                             NULL
                        };

static void _bc_cmwx1zzabz_task(void *param);

static bool _bc_cmwx1zzabz_read_response(bc_cmwx1zzabz_t *self);

void bc_cmwx1zzabz_init(bc_cmwx1zzabz_t *self,  bc_uart_channel_t uart_channel)
{
    memset(self, 0, sizeof(*self));

    self->_uart_channel = uart_channel;

    bc_fifo_init(&self->_tx_fifo, self->_tx_fifo_buffer, sizeof(self->_tx_fifo_buffer));
    bc_fifo_init(&self->_rx_fifo, self->_rx_fifo_buffer, sizeof(self->_rx_fifo_buffer));

    bc_uart_init(self->_uart_channel, BC_UART_BAUDRATE_9600, BC_UART_SETTING_8N1);
    bc_uart_set_async_fifo(self->_uart_channel, &self->_tx_fifo, &self->_rx_fifo);
    bc_uart_async_read_start(self->_uart_channel, BC_TICK_INFINITY);

    self->_task_id = bc_scheduler_register(_bc_cmwx1zzabz_task, self, BC_CMWX1ZZABZ_DELAY_RUN);
    self->_save_flag = false;
    self->_join_command = false;
    self->_save_config_mask = 0x00;
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

bool bc_cmwx1zzabz_send_message(bc_cmwx1zzabz_t *self, const void *buffer, size_t length)
{
    if (!bc_cmwx1zzabz_is_ready(self) || length == 0 || length > 51)
    {
        return false;
    }

    self->_message_length = length;

    memcpy(self->_message_buffer, buffer, self->_message_length);

    self->_state = BC_CMWX1ZZABZ_STATE_SEND_MESSAGE_COMMAND;

    bc_scheduler_plan_now(self->_task_id);

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
                if(self->_join_command)
                {
                    self->_state = BC_CMWX1ZZABZ_STATE_JOIN_SEND;
                    continue;
                }

                return;
            }
            case BC_CMWX1ZZABZ_STATE_ERROR:
            {
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
                while (bc_uart_async_read(self->_uart_channel, &rx_character, 1) != 0)
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
                uint8_t response_handled = 0;

                if (!_bc_cmwx1zzabz_read_response(self))
                {
                    continue;
                }

                // Compare first 4 cahracters from response
                uint32_t response_valid = (memcmp(self->_response, "+OK=", 4) == 0);
                // Pointer to the last send command to know the context of the answer
                const char *last_command = _init_commands[self->init_command_index];
                // Pointer to the first character of response value after +OK=
                char *response_string_value = &self->_response[4];
                
                if (strcmp(last_command, "AT+DEVADDR?\r") == 0 && response_valid)
                {
                    // Check if user did not filled this structure to save configuration, oterwise it would be overwritten
                    if ((self->_save_config_mask & 1 << BC_CMWX1ZZABZ_CONFIG_INDEX_DEVADDR) == 0)
                    {
                        memcpy(self->_config.devaddr, response_string_value, 8);
                        self->_config.devaddr[8] = '\0';
                    }
                    response_handled = 1;
                }
                else if (strcmp(last_command, "AT+DEVEUI?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << BC_CMWX1ZZABZ_CONFIG_INDEX_DEVEUI) == 0)
                    {
                        memcpy(self->_config.deveui, response_string_value, 16);
                        self->_config.deveui[16] = '\0';
                    }
                    response_handled = 1;
                } 
                else if (strcmp(last_command, "AT+APPEUI?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << BC_CMWX1ZZABZ_CONFIG_INDEX_APPEUI) == 0)
                    {
                        memcpy(self->_config.appeui, response_string_value, 16);
                        self->_config.appeui[16] = '\0';
                    }
                    response_handled = 1;
                } 
                else if (strcmp(last_command, "AT+NWKSKEY?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << BC_CMWX1ZZABZ_CONFIG_INDEX_NWKSKEY) == 0)
                    {
                        memcpy(self->_config.nwkskey, response_string_value, 32);
                        self->_config.nwkskey[32] = '\0';
                    }
                    response_handled = 1;
                } 
                else if (strcmp(last_command, "AT+APPSKEY?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << BC_CMWX1ZZABZ_CONFIG_INDEX_APPSKEY) == 0)
                    {
                        memcpy(self->_config.appskey, response_string_value, 32);
                        self->_config.appskey[32] = '\0';
                    }
                    response_handled = 1;
                } 
                else if (strcmp(last_command, "AT+APPKEY?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << BC_CMWX1ZZABZ_CONFIG_INDEX_APPKEY) == 0)
                    {
                        memcpy(self->_config.appkey, response_string_value, 32);
                        self->_config.appkey[32] = '\0';
                    }
                    response_handled = 1;
                } 
                else if (strcmp(last_command, "AT+BAND?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << BC_CMWX1ZZABZ_CONFIG_INDEX_BAND) == 0)
                    {
                        self->_config.band = response_string_value[0] - '0';
                    }
                    response_handled = 1;
                } 
                else if (strcmp(last_command, "AT+MODE?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << BC_CMWX1ZZABZ_CONFIG_INDEX_MODE) == 0)
                    {
                        self->_config.mode = response_string_value[0] - '0';
                    }
                    response_handled = 1;
                }
                else if (   strcmp(last_command, "AT\r") == 0 &&
                            strcmp(self->_response, "+OK\r") == 0
                        )
                {
                    response_handled = 1;
                }
                // Generic OK response to other commands
                else if (memcmp(self->_response, "+OK", 3) == 0)
                {
                    response_handled = 1;
                }

                if (!response_handled)
                {
                    continue;
                }

                self->init_command_index++;

                if (_init_commands[self->init_command_index] == NULL)
                {
                    // If configuration was changed and flag set, save them
                    if (self->_save_config_mask)
                    {
                        self->_state = BC_CMWX1ZZABZ_STATE_CONFIG_SAVE_SEND;
                        self->_save_command_index = 0;
                    }
                    else
                    {
                        self->_state = BC_CMWX1ZZABZ_STATE_READY;
                    }
                }
                else
                {
                    self->_state = BC_CMWX1ZZABZ_STATE_INITIALIZE_COMMAND_SEND;
                }

                continue;
            }
            case BC_CMWX1ZZABZ_STATE_SEND_MESSAGE_COMMAND:
            {
                self->_state = BC_CMWX1ZZABZ_STATE_ERROR;

                snprintf(self->_command, BC_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+UTX %d\r", self->_message_length);

                uint8_t command_length = strlen(self->_command);

                for (size_t i = 0; i < self->_message_length; i++)
                {
                    // put binary data directly to the "string" buffer
                    self->_command[command_length + i] = self->_message_buffer[i];
                }

                self->_command[command_length + self->_message_length] = '\r';

                size_t length = command_length + self->_message_length + 1; // 1 for \n

                if (bc_uart_async_write(self->_uart_channel, self->_command, length) != length)
                {
                    continue;
                }

                self->_state = BC_CMWX1ZZABZ_STATE_SEND_MESSAGE_RESPONSE;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_CMWX1ZZABZ_EVENT_SEND_RF_FRAME_START, self->_event_param);
                }

                bc_scheduler_plan_current_from_now(BC_CMWX1ZZABZ_DELAY_SEND_RF_FRAME_RESPONSE);

                return;
            }
            case BC_CMWX1ZZABZ_STATE_SEND_MESSAGE_RESPONSE:
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

            case BC_CMWX1ZZABZ_STATE_CONFIG_SAVE_SEND:
            {
                self->_state = BC_CMWX1ZZABZ_STATE_ERROR;

                // There are no more config items to send
                if (self->_save_config_mask == 0)
                {
                    if (self->_event_handler != NULL)
                    {
                        self->_event_handler(self, BC_CMWX1ZZABZ_EVENT_CONFIG_SAVE_DONE, self->_event_param);
                    }

                    self->_state = BC_CMWX1ZZABZ_STATE_READY;
                    continue;
                }

                // Find config item that has been changed
                uint8_t i;
                for (i = 0; i < BC_CMWX1ZZABZ_CONFIG_INDEX_LAST_ITEM; i++)
                { 
                    if (self->_save_config_mask & 1 << i)
                    {
                        self->_save_command_index = i;
                        break;
                    }
                }

                // Purge RX FIFO
                char rx_character;
                while (bc_uart_async_read(self->_uart_channel, &rx_character, 1) != 0)
                {
                }

                switch (self->_save_command_index)
                {
                    case BC_CMWX1ZZABZ_CONFIG_INDEX_DEVADDR:
                    {
                        snprintf(self->_command, BC_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+DEVADDR=%s\r", self->_config.devaddr);
                        break;
                    }
                    case BC_CMWX1ZZABZ_CONFIG_INDEX_DEVEUI:
                    {
                        snprintf(self->_command, BC_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+DEVEUI=%s\r", self->_config.deveui);
                        break;
                    }
                    case BC_CMWX1ZZABZ_CONFIG_INDEX_APPEUI:
                    {
                        snprintf(self->_command, BC_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+APPEUI=%s\r", self->_config.appeui);
                        break;
                    }
                    case BC_CMWX1ZZABZ_CONFIG_INDEX_NWKSKEY:
                    {
                        snprintf(self->_command, BC_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+NWKSKEY=%s\r", self->_config.nwkskey);
                        break;
                    }
                    case BC_CMWX1ZZABZ_CONFIG_INDEX_APPSKEY:
                    {
                        snprintf(self->_command, BC_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+APPSKEY=%s\r", self->_config.appskey);
                        break;
                    }
                    case BC_CMWX1ZZABZ_CONFIG_INDEX_APPKEY:
                    {
                        snprintf(self->_command, BC_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+APPKEY=%s\r", self->_config.appkey);
                        break;
                    }
                    case BC_CMWX1ZZABZ_CONFIG_INDEX_BAND:
                    {
                        snprintf(self->_command, BC_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+BAND=%d\r", self->_config.band);
                        break;
                    }
                    case BC_CMWX1ZZABZ_CONFIG_INDEX_MODE:
                    {
                        snprintf(self->_command, BC_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+MODE=%d\r", self->_config.mode);
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }

                size_t length = strlen(self->_command);

                if (bc_uart_async_write(self->_uart_channel, self->_command, length) != length)
                {
                    continue;
                }
        
                self->_state = BC_CMWX1ZZABZ_STATE_CONFIG_SAVE_RESPONSE;
                bc_scheduler_plan_current_from_now(BC_CMWX1ZZABZ_DELAY_INITIALIZATION_AT_COMMAND);
                return;
            }

            case BC_CMWX1ZZABZ_STATE_CONFIG_SAVE_RESPONSE:
            {
                self->_state = BC_CMWX1ZZABZ_STATE_ERROR;

                if (!_bc_cmwx1zzabz_read_response(self))
                {
                    continue;
                }

                // Jump to error state when response is not OK
                if (memcmp(self->_response, "+OK", 3) != 0)
                {
                    continue;
                }

                // Clean bit mask
                self->_save_config_mask &= ~(1 << self->_save_command_index);

                self->_state = BC_CMWX1ZZABZ_STATE_CONFIG_SAVE_SEND;
                continue;

            }


            case BC_CMWX1ZZABZ_STATE_JOIN_SEND:
            {
                self->_state = BC_CMWX1ZZABZ_STATE_ERROR;

                // Purge RX FIFO
                char rx_character;
                while (bc_uart_async_read(self->_uart_channel, &rx_character, 1) != 0)
                {
                }

                strcpy(self->_command, "AT+JOIN\r");

                size_t length = strlen(self->_command);
                if (bc_uart_async_write(self->_uart_channel, self->_command, length) != length)
                {
                    continue;
                }
        
                self->_state = BC_CMWX1ZZABZ_STATE_JOIN_RESPONSE;
                bc_scheduler_plan_current_from_now(BC_CMWX1ZZABZ_DELAY_JOIN_RESPONSE);
                return;
            }

            case BC_CMWX1ZZABZ_STATE_JOIN_RESPONSE:
            {
                bool join_successful = false;
                // Clean join command flag
                self->_join_command = false;

                while (true)
                {
                    if (!_bc_cmwx1zzabz_read_response(self))
                    {
                        break;
                    }

                    // Response EVENT=1,1 means JOIN was successful
                    if (memcmp(self->_response, "+EVENT=1,1", 10) == 0)
                    {
                        join_successful = true;
                        break;
                    }
                }
                
                if (join_successful)
                {
                    if (self->_event_handler != NULL)
                    {
                        self->_event_handler(self, BC_CMWX1ZZABZ_EVENT_JOIN_SUCCESS, self->_event_param);
                    }
                }
                else 
                {
                    if (self->_event_handler != NULL)
                    {
                        self->_event_handler(self, BC_CMWX1ZZABZ_EVENT_JOIN_ERROR, self->_event_param);
                    }
                }               
                
                self->_state = BC_CMWX1ZZABZ_STATE_IDLE;
                continue;
            }

            default:
            {
                break;
            }
        }
    }
}

void bc_cmwx1zzabz_join(bc_cmwx1zzabz_t *self)
{
    self->_join_command = true;
    bc_scheduler_plan_now(self->_task_id);
}

void bc_cmwx1zzabz_set_devaddr(bc_cmwx1zzabz_t *self, char *devaddr)
{
    strncpy(self->_config.devaddr, devaddr, 8+1);
    self->_save_config_mask |= (1 << BC_CMWX1ZZABZ_CONFIG_INDEX_DEVADDR);
}

void bc_cmwx1zzabz_get_devaddr(bc_cmwx1zzabz_t *self, char *devaddr)
{
    strncpy(devaddr, self->_config.devaddr, 8+1);
}

void bc_cmwx1zzabz_set_deveui(bc_cmwx1zzabz_t *self, char *deveui)
{
    strncpy(self->_config.deveui, deveui, 16+1);
    self->_save_config_mask |= (1 << BC_CMWX1ZZABZ_CONFIG_INDEX_DEVEUI);
}

void bc_cmwx1zzabz_get_deveui(bc_cmwx1zzabz_t *self, char *deveui)
{
    strncpy(deveui, self->_config.deveui, 16+1);
}

void bc_cmwx1zzabz_set_appeui(bc_cmwx1zzabz_t *self, char *appeui)
{
    strncpy(self->_config.appeui, appeui, 16+1);
    self->_save_config_mask |= (1 << BC_CMWX1ZZABZ_CONFIG_INDEX_APPEUI);
}

void bc_cmwx1zzabz_get_appeui(bc_cmwx1zzabz_t *self, char *appeui)
{
    strncpy(appeui, self->_config.appeui, 16+1);
}

void bc_cmwx1zzabz_set_nwkskey(bc_cmwx1zzabz_t *self, char *nwkskey)
{
    strncpy(self->_config.nwkskey, nwkskey, 32);
    self->_save_config_mask |= (1 << BC_CMWX1ZZABZ_CONFIG_INDEX_NWKSKEY);
}

void bc_cmwx1zzabz_get_nwkskey(bc_cmwx1zzabz_t *self, char *nwkskey)
{
    strncpy(nwkskey, self->_config.nwkskey, 32+1);
}

void bc_cmwx1zzabz_set_appskey(bc_cmwx1zzabz_t *self, char *appskey)
{
    strncpy(self->_config.appskey, appskey, 32);
    self->_save_config_mask |= (1 << BC_CMWX1ZZABZ_CONFIG_INDEX_APPSKEY);
}

void bc_cmwx1zzabz_get_appskey(bc_cmwx1zzabz_t *self, char *appskey)
{
    strncpy(appskey, self->_config.appskey, 32+1);
}

void bc_cmwx1zzabz_set_appkey(bc_cmwx1zzabz_t *self, char *appkey)
{
    strncpy(self->_config.appkey, appkey, 32+1);
    self->_save_config_mask |= (1 << BC_CMWX1ZZABZ_CONFIG_INDEX_APPKEY);
}

void bc_cmwx1zzabz_get_appkey(bc_cmwx1zzabz_t *self, char *appkey)
{
    strncpy(appkey, self->_config.appkey, 32+1);
}

void bc_cmwx1zzabz_set_band(bc_cmwx1zzabz_t *self, bc_cmwx1zzabz_config_band_t band)
{
    self->_config.band = band;
    self->_save_config_mask |= (1 << BC_CMWX1ZZABZ_CONFIG_INDEX_BAND);
}

bc_cmwx1zzabz_config_band_t bc_cmwx1zzabz_get_band(bc_cmwx1zzabz_t *self)
{
    return self->_config.band;
}

void bc_cmwx1zzabz_set_mode(bc_cmwx1zzabz_t *self, bc_cmwx1zzabz_config_mode_t mode)
{
    self->_config.mode = mode;
    self->_save_config_mask |= (1 << BC_CMWX1ZZABZ_CONFIG_INDEX_MODE);
}

bc_cmwx1zzabz_config_mode_t bc_cmwx1zzabz_get_mode(bc_cmwx1zzabz_t *self)
{
    return self->_config.mode;
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
