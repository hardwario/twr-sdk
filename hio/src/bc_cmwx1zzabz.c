#include <hio_cmwx1zzabz.h>
#include <hio_log.h>

#define HIO_CMWX1ZZABZ_DELAY_RUN 100
#define HIO_CMWX1ZZABZ_DELAY_INITIALIZATION_RESET_H 100
#define HIO_CMWX1ZZABZ_DELAY_INITIALIZATION_AT_COMMAND 100 // ! when using longer AT responses
#define HIO_CMWX1ZZABZ_DELAY_INITIALIZATION_AT_RESPONSE 100
#define HIO_CMWX1ZZABZ_DELAY_SEND_MESSAGE_RESPONSE 3000
#define HIO_CMWX1ZZABZ_DELAY_JOIN_RESPONSE 8000

// Apply changes to the factory configuration
const char *_init_commands[] =
{
    "\rAT\r",
    "AT+VER?\r",
    "AT+DFORMAT=0\r",
    "AT+DUTYCYCLE=0\r",
    "AT+DEVADDR?\r",
    "AT+DEVEUI?\r",
    "AT+APPEUI?\r",
    "AT+NWKSKEY?\r",
    "AT+APPSKEY?\r",
    "AT+APPKEY?\r",
    "AT+BAND?\r",
    "AT+MODE?\r",
    "AT+CLASS?\r",
    "AT+RX2?\r",
    "AT+NWK?\r",
    "AT+DR?\r",
    NULL
};

static void _hio_cmwx1zzabz_task(void *param);

static bool _hio_cmwx1zzabz_read_response(hio_cmwx1zzabz_t *self);

static void _hio_cmwx1zzabz_save_config(hio_cmwx1zzabz_t *self, hio_cmwx1zzabz_config_index_t config_index);

static void _uart_event_handler(hio_uart_channel_t channel, hio_uart_event_t event, void *param);

void hio_cmwx1zzabz_init(hio_cmwx1zzabz_t *self,  hio_uart_channel_t uart_channel)
{
    memset(self, 0, sizeof(*self));

    self->_uart_channel = uart_channel;
    self->_tx_port = 2;

    hio_fifo_init(&self->_tx_fifo, self->_tx_fifo_buffer, sizeof(self->_tx_fifo_buffer));
    hio_fifo_init(&self->_rx_fifo, self->_rx_fifo_buffer, sizeof(self->_rx_fifo_buffer));

    hio_uart_init(self->_uart_channel, HIO_UART_BAUDRATE_9600, HIO_UART_SETTING_8N1);
    hio_uart_set_async_fifo(self->_uart_channel, &self->_tx_fifo, &self->_rx_fifo);
    hio_uart_async_read_start(self->_uart_channel, HIO_TICK_INFINITY);
    hio_uart_set_event_handler(self->_uart_channel, _uart_event_handler, self);

    self->_task_id = hio_scheduler_register(_hio_cmwx1zzabz_task, self, HIO_CMWX1ZZABZ_DELAY_RUN);
    self->_state = HIO_CMWX1ZZABZ_STATE_INITIALIZE;
}

static void _uart_event_handler(hio_uart_channel_t channel, hio_uart_event_t event, void *param)
{
    (void) channel;
    hio_cmwx1zzabz_t *self = (hio_cmwx1zzabz_t*)param;

    if (event == HIO_UART_EVENT_ASYNC_READ_DATA && self->_state == HIO_CMWX1ZZABZ_STATE_IDLE)
    {
        hio_scheduler_plan_relative(self->_task_id, 100);
        self->_state = HIO_CMWX1ZZABZ_STATE_RECEIVE;
    }
}

void hio_cmwx1zzabz_set_event_handler(hio_cmwx1zzabz_t *self, void (*event_handler)(hio_cmwx1zzabz_t *, hio_cmwx1zzabz_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

bool hio_cmwx1zzabz_is_ready(hio_cmwx1zzabz_t *self)
{
    if (self->_state == HIO_CMWX1ZZABZ_STATE_IDLE)
    {
        return true;
    }

    return false;
}

bool hio_cmwx1zzabz_send_message(hio_cmwx1zzabz_t *self, const void *buffer, size_t length)
{
    if (!hio_cmwx1zzabz_is_ready(self) || length == 0 || length > HIO_CMWX1ZZABZ_TX_MAX_PACKET_SIZE)
    {
        return false;
    }

    self->_message_length = length;

    memcpy(self->_message_buffer, buffer, self->_message_length);

    self->_state = HIO_CMWX1ZZABZ_STATE_SEND_MESSAGE_COMMAND;

    hio_scheduler_plan_now(self->_task_id);

    return true;
}

bool hio_cmwx1zzabz_send_message_confirmed(hio_cmwx1zzabz_t *self, const void *buffer, size_t length)
{
    if (!hio_cmwx1zzabz_is_ready(self) || length == 0 || length > HIO_CMWX1ZZABZ_TX_MAX_PACKET_SIZE)
    {
        return false;
    }

    self->_message_length = length;

    memcpy(self->_message_buffer, buffer, self->_message_length);

    self->_state = HIO_CMWX1ZZABZ_STATE_SEND_MESSAGE_CONFIRMED_COMMAND;

    hio_scheduler_plan_now(self->_task_id);

    return true;
}

void hio_cmwx1zzabz_set_debug(hio_cmwx1zzabz_t *self, bool debug)
{
    self->_debug = debug;
}

static size_t _hio_cmwx1zzabz_async_write(hio_cmwx1zzabz_t *self, hio_uart_channel_t channel, const void *buffer, size_t length)
{
    size_t ret = hio_uart_async_write(channel, buffer, length);

    if (self->_debug)
    {
        hio_log_debug("LoRa TX: %s", (const char*)buffer);
    }

    return ret;
}

static void _hio_cmwx1zzabz_task(void *param)
{
    hio_cmwx1zzabz_t *self = param;

    while (true)
    {
        switch (self->_state)
        {
            case HIO_CMWX1ZZABZ_STATE_READY:
            {
                self->_state = HIO_CMWX1ZZABZ_STATE_IDLE;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, HIO_CMWX1ZZABZ_EVENT_READY, self->_event_param);
                }

                continue;
            }
            case HIO_CMWX1ZZABZ_STATE_IDLE:
            {
                if (self->_save_config_mask != 0)
                {
                    self->_state = HIO_CMWX1ZZABZ_STATE_CONFIG_SAVE_SEND;
                    continue;
                }

                if(self->_join_command)
                {
                    self->_state = HIO_CMWX1ZZABZ_STATE_JOIN_SEND;
                    continue;
                }

                return;
            }
            case HIO_CMWX1ZZABZ_STATE_RECEIVE:
            {
                self->_state = HIO_CMWX1ZZABZ_STATE_IDLE;

                while (_hio_cmwx1zzabz_read_response(self))
                {
                    if (memcmp(self->_response, "+RECV=", 5) == 0)
                    {
                        self->_message_port = atoi(&self->_response[6]);

                        char *comma_search = strchr(self->_response, ',');
                        if (!comma_search)
                        {
                            continue;
                        }

                        // Parse from the next character
                        self->_message_length = atoi(++comma_search);

                        // Dummy read three \r\n\r characters
                        char dummy[3];
                        uint32_t bytes = hio_uart_async_read(self->_uart_channel, &dummy, 3);
                        if (bytes != 3)
                        {
                            continue;
                        }

                        // Received data is bigger than library message buffer
                        if (self->_message_length > sizeof(self->_message_buffer))
                        {
                            continue;
                        }

                        // Read the received message
                        bytes = hio_uart_async_read(self->_uart_channel, self->_message_buffer, self->_message_length);
                        if (bytes != self->_message_length)
                        {
                            continue;
                        }

                        if (self->_event_handler != NULL)
                        {
                            self->_event_handler(self, HIO_CMWX1ZZABZ_EVENT_MESSAGE_RECEIVED, self->_event_param);
                        }
                    }
                    else if (memcmp(self->_response, "+ACK", 4) == 0)
                    {
                        if (self->_event_handler != NULL)
                        {
                            self->_event_handler(self, HIO_CMWX1ZZABZ_EVENT_MESSAGE_CONFIRMED, self->_event_param);
                        }
                    }
                    else if (memcmp(self->_response, "+NOACK", 4) == 0)
                    {
                        if (self->_event_handler != NULL)
                        {
                            self->_event_handler(self, HIO_CMWX1ZZABZ_EVENT_MESSAGE_NOT_CONFIRMED, self->_event_param);
                        }
                    }
                    else if (memcmp(self->_response, "+EVENT=2,2", 10) == 0)
                    {
                        if (self->_event_handler != NULL)
                        {
                            self->_event_handler(self, HIO_CMWX1ZZABZ_EVENT_MESSAGE_RETRANSMISSION, self->_event_param);
                        }
                    }
                }

                return;
            }
            case HIO_CMWX1ZZABZ_STATE_ERROR:
            {
                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, HIO_CMWX1ZZABZ_EVENT_ERROR, self->_event_param);
                }

                self->_state = HIO_CMWX1ZZABZ_STATE_INITIALIZE;

                continue;
            }
            case HIO_CMWX1ZZABZ_STATE_INITIALIZE:
            {
                self->_init_command_index = 0;
                self->_state = HIO_CMWX1ZZABZ_STATE_INITIALIZE_COMMAND_SEND;

                continue;
            }

            case HIO_CMWX1ZZABZ_STATE_INITIALIZE_COMMAND_SEND:
            {
                self->_state = HIO_CMWX1ZZABZ_STATE_ERROR;

                // Purge RX FIFO
                char rx_character;
                while (hio_uart_async_read(self->_uart_channel, &rx_character, 1) != 0)
                {
                }

                strcpy(self->_command, _init_commands[self->_init_command_index]);
                size_t length = strlen(self->_command);

                if (_hio_cmwx1zzabz_async_write(self, self->_uart_channel, self->_command, length) != length)
                {
                    continue;
                }

                self->_state = HIO_CMWX1ZZABZ_STATE_INITIALIZE_COMMAND_RESPONSE;
                hio_scheduler_plan_current_from_now(HIO_CMWX1ZZABZ_DELAY_INITIALIZATION_AT_COMMAND);

                return;
            }
            case HIO_CMWX1ZZABZ_STATE_INITIALIZE_COMMAND_RESPONSE:
            {
                self->_state = HIO_CMWX1ZZABZ_STATE_ERROR;
                uint8_t response_handled = 0;

                if (!_hio_cmwx1zzabz_read_response(self))
                {
                    continue;
                }

                // Compare first 4 cahracters from response
                uint32_t response_valid = (memcmp(self->_response, "+OK=", 4) == 0);
                // Pointer to the last send command to know the context of the answer
                const char *last_command = _init_commands[self->_init_command_index];
                // Pointer to the first character of response value after +OK=
                char *response_string_value = &self->_response[4];

                if (strcmp(last_command, "AT+DEVADDR?\r") == 0 && response_valid)
                {
                    // Check if user did not filled this structure to save configuration, oterwise it would be overwritten
                    if ((self->_save_config_mask & 1 << HIO_CMWX1ZZABZ_CONFIG_INDEX_DEVADDR) == 0)
                    {
                        memcpy(self->_config.devaddr, response_string_value, 8);
                        self->_config.devaddr[8] = '\0';
                    }
                    response_handled = 1;
                }
                else if (strcmp(last_command, "AT+DEVEUI?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << HIO_CMWX1ZZABZ_CONFIG_INDEX_DEVEUI) == 0)
                    {
                        memcpy(self->_config.deveui, response_string_value, 16);
                        self->_config.deveui[16] = '\0';
                    }
                    response_handled = 1;
                }
                else if (strcmp(last_command, "AT+APPEUI?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << HIO_CMWX1ZZABZ_CONFIG_INDEX_APPEUI) == 0)
                    {
                        memcpy(self->_config.appeui, response_string_value, 16);
                        self->_config.appeui[16] = '\0';
                    }
                    response_handled = 1;
                }
                else if (strcmp(last_command, "AT+NWKSKEY?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << HIO_CMWX1ZZABZ_CONFIG_INDEX_NWKSKEY) == 0)
                    {
                        memcpy(self->_config.nwkskey, response_string_value, 32);
                        self->_config.nwkskey[32] = '\0';
                    }
                    response_handled = 1;
                }
                else if (strcmp(last_command, "AT+APPSKEY?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << HIO_CMWX1ZZABZ_CONFIG_INDEX_APPSKEY) == 0)
                    {
                        memcpy(self->_config.appskey, response_string_value, 32);
                        self->_config.appskey[32] = '\0';
                    }
                    response_handled = 1;
                }
                else if (strcmp(last_command, "AT+APPKEY?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << HIO_CMWX1ZZABZ_CONFIG_INDEX_APPKEY) == 0)
                    {
                        memcpy(self->_config.appkey, response_string_value, 32);
                        self->_config.appkey[32] = '\0';
                    }
                    response_handled = 1;
                }
                else if (strcmp(last_command, "AT+BAND?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << HIO_CMWX1ZZABZ_CONFIG_INDEX_BAND) == 0)
                    {
                        self->_config.band = response_string_value[0] - '0';
                    }
                    response_handled = 1;
                }
                else if (strcmp(last_command, "AT+MODE?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << HIO_CMWX1ZZABZ_CONFIG_INDEX_MODE) == 0)
                    {
                        self->_config.mode = response_string_value[0] - '0';
                    }
                    response_handled = 1;
                }
                else if (strcmp(last_command, "AT+CLASS?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << HIO_CMWX1ZZABZ_CONFIG_INDEX_CLASS) == 0)
                    {
                        self->_config.class = response_string_value[0] - '0';
                    }
                    response_handled = 1;
                }
                else if (strcmp(last_command, "AT+RX2?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << HIO_CMWX1ZZABZ_CONFIG_INDEX_RX2) == 0)
                    {
                        self->_config.rx2_frequency = atoi(response_string_value);

                        char *comma_search = strchr(response_string_value, ',');
                        if (!comma_search)
                        {
                            continue;
                        }

                        self->_config.rx2_datarate = atoi(++comma_search);
                    }
                    response_handled = 1;
                }
                else if (strcmp(last_command, "AT+NWK?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << HIO_CMWX1ZZABZ_CONFIG_INDEX_NWK) == 0)
                    {
                        self->_config.nwk_public = response_string_value[0] - '0';
                    }
                    response_handled = 1;
                }
                else if (strcmp(last_command, "AT+DR?\r") == 0 && response_valid)
                {
                    if ((self->_save_config_mask & 1 << HIO_CMWX1ZZABZ_CONFIG_INDEX_DATARATE) == 0)
                    {
                        self->_config.datarate = atoi(response_string_value);
                    }
                    response_handled = 1;
                }
                else if (strcmp(last_command, "AT+DUTYCYCLE=0\r") == 0 && strcmp(self->_response, "+ERR=-17\r") == 0)
                {
                    // DUTYCYLE is unusable in some band configuration, ignore this err response
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

                self->_init_command_index++;

                if (_init_commands[self->_init_command_index] == NULL)
                {
                    // If configuration was changed and flag set, save them
                    if (self->_save_config_mask)
                    {
                        self->_state = HIO_CMWX1ZZABZ_STATE_CONFIG_SAVE_SEND;
                        self->_save_command_index = 0;
                    }
                    else
                    {
                        self->_state = HIO_CMWX1ZZABZ_STATE_READY;
                    }
                }
                else
                {
                    self->_state = HIO_CMWX1ZZABZ_STATE_INITIALIZE_COMMAND_SEND;
                }

                continue;
            }
            case HIO_CMWX1ZZABZ_STATE_SEND_MESSAGE_COMMAND:
            case HIO_CMWX1ZZABZ_STATE_SEND_MESSAGE_CONFIRMED_COMMAND:
            {
                if (self->_state == HIO_CMWX1ZZABZ_STATE_SEND_MESSAGE_CONFIRMED_COMMAND)
                {
                    snprintf(self->_command, HIO_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+PCTX %d,%d\r", self->_tx_port, self->_message_length);
                }
                else
                {
                    snprintf(self->_command, HIO_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+PUTX %d,%d\r", self->_tx_port, self->_message_length);
                }

                self->_state = HIO_CMWX1ZZABZ_STATE_ERROR;

                uint8_t command_length = strlen(self->_command);

                for (size_t i = 0; i < self->_message_length; i++)
                {
                    // put binary data directly to the "string" buffer
                    self->_command[command_length + i] = self->_message_buffer[i];
                }

                self->_command[command_length + self->_message_length] = '\r';

                size_t length = command_length + self->_message_length + 1; // 1 for \n

                if (_hio_cmwx1zzabz_async_write(self, self->_uart_channel, self->_command, length) != length)
                {
                    continue;
                }

                self->_state = HIO_CMWX1ZZABZ_STATE_SEND_MESSAGE_RESPONSE;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, HIO_CMWX1ZZABZ_EVENT_SEND_MESSAGE_START, self->_event_param);
                }

                hio_scheduler_plan_current_from_now(HIO_CMWX1ZZABZ_DELAY_SEND_MESSAGE_RESPONSE);

                return;
            }
            case HIO_CMWX1ZZABZ_STATE_SEND_MESSAGE_RESPONSE:
            {
                self->_state = HIO_CMWX1ZZABZ_STATE_ERROR;

                if (!_hio_cmwx1zzabz_read_response(self))
                {
                    continue;
                }

                if (strcmp(self->_response, "+OK\r") != 0)
                {
                    continue;
                }

                self->_state = HIO_CMWX1ZZABZ_STATE_READY;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, HIO_CMWX1ZZABZ_EVENT_SEND_MESSAGE_DONE, self->_event_param);
                }

                continue;
            }
            case HIO_CMWX1ZZABZ_STATE_CONFIG_SAVE_SEND:
            {
                self->_state = HIO_CMWX1ZZABZ_STATE_ERROR;

                // There are no more config items to send
                if (self->_save_config_mask == 0)
                {
                    if (self->_event_handler != NULL)
                    {
                        self->_event_handler(self, HIO_CMWX1ZZABZ_EVENT_CONFIG_SAVE_DONE, self->_event_param);
                    }

                    self->_state = HIO_CMWX1ZZABZ_STATE_READY;
                    continue;
                }

                // Find config item that has been changed
                for (uint8_t i = 0; i < HIO_CMWX1ZZABZ_CONFIG_INDEX_LAST_ITEM; i++)
                {
                    if (self->_save_config_mask & 1 << i)
                    {
                        self->_save_command_index = i;
                        break;
                    }
                }

                // Purge RX FIFO
                char rx_character;
                while (hio_uart_async_read(self->_uart_channel, &rx_character, 1) != 0)
                {
                }

                switch (self->_save_command_index)
                {
                    case HIO_CMWX1ZZABZ_CONFIG_INDEX_DEVADDR:
                    {
                        snprintf(self->_command, HIO_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+DEVADDR=%s\r", self->_config.devaddr);
                        break;
                    }
                    case HIO_CMWX1ZZABZ_CONFIG_INDEX_DEVEUI:
                    {
                        snprintf(self->_command, HIO_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+DEVEUI=%s\r", self->_config.deveui);
                        break;
                    }
                    case HIO_CMWX1ZZABZ_CONFIG_INDEX_APPEUI:
                    {
                        snprintf(self->_command, HIO_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+APPEUI=%s\r", self->_config.appeui);
                        break;
                    }
                    case HIO_CMWX1ZZABZ_CONFIG_INDEX_NWKSKEY:
                    {
                        snprintf(self->_command, HIO_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+NWKSKEY=%s\r", self->_config.nwkskey);
                        break;
                    }
                    case HIO_CMWX1ZZABZ_CONFIG_INDEX_APPSKEY:
                    {
                        snprintf(self->_command, HIO_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+APPSKEY=%s\r", self->_config.appskey);
                        break;
                    }
                    case HIO_CMWX1ZZABZ_CONFIG_INDEX_APPKEY:
                    {
                        snprintf(self->_command, HIO_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+APPKEY=%s\r", self->_config.appkey);
                        break;
                    }
                    case HIO_CMWX1ZZABZ_CONFIG_INDEX_BAND:
                    {
                        snprintf(self->_command, HIO_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+BAND=%d\r", self->_config.band);
                        break;
                    }
                    case HIO_CMWX1ZZABZ_CONFIG_INDEX_MODE:
                    {
                        snprintf(self->_command, HIO_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+MODE=%d\r", self->_config.mode);
                        break;
                    }
                    case HIO_CMWX1ZZABZ_CONFIG_INDEX_CLASS:
                    {
                        snprintf(self->_command, HIO_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+CLASS=%d\r", self->_config.class);
                        break;
                    }
                    case HIO_CMWX1ZZABZ_CONFIG_INDEX_RX2:
                    {
                        snprintf(self->_command, HIO_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+RX2=%d,%d\r", (int) self->_config.rx2_frequency, self->_config.rx2_datarate);
                        break;
                    }
                    case HIO_CMWX1ZZABZ_CONFIG_INDEX_NWK:
                    {
                        snprintf(self->_command, HIO_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+NWK=%d\r", (int) self->_config.nwk_public);
                        break;
                    }
                    case HIO_CMWX1ZZABZ_CONFIG_INDEX_DATARATE:
                    {
                        snprintf(self->_command, HIO_CMWX1ZZABZ_TX_FIFO_BUFFER_SIZE, "AT+DR=%d\r", (int) self->_config.datarate);
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }

                size_t length = strlen(self->_command);

                if (_hio_cmwx1zzabz_async_write(self, self->_uart_channel, self->_command, length) != length)
                {
                    continue;
                }

                self->_state = HIO_CMWX1ZZABZ_STATE_CONFIG_SAVE_RESPONSE;
                hio_scheduler_plan_current_from_now(HIO_CMWX1ZZABZ_DELAY_INITIALIZATION_AT_COMMAND);
                return;
            }

            case HIO_CMWX1ZZABZ_STATE_CONFIG_SAVE_RESPONSE:
            {
                self->_state = HIO_CMWX1ZZABZ_STATE_ERROR;

                if (!_hio_cmwx1zzabz_read_response(self))
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

                self->_state = HIO_CMWX1ZZABZ_STATE_CONFIG_SAVE_SEND;
                continue;

            }


            case HIO_CMWX1ZZABZ_STATE_JOIN_SEND:
            {
                self->_state = HIO_CMWX1ZZABZ_STATE_ERROR;

                // Purge RX FIFO
                char rx_character;
                while (hio_uart_async_read(self->_uart_channel, &rx_character, 1) != 0)
                {
                }

                strcpy(self->_command, "AT+JOIN\r");

                size_t length = strlen(self->_command);
                if (_hio_cmwx1zzabz_async_write(self, self->_uart_channel, self->_command, length) != length)
                {
                    continue;
                }

                self->_state = HIO_CMWX1ZZABZ_STATE_JOIN_RESPONSE;
                hio_scheduler_plan_current_from_now(HIO_CMWX1ZZABZ_DELAY_JOIN_RESPONSE);
                return;
            }

            case HIO_CMWX1ZZABZ_STATE_JOIN_RESPONSE:
            {
                bool join_successful = false;
                // Clear join command flag
                self->_join_command = false;

                while (true)
                {
                    if (!_hio_cmwx1zzabz_read_response(self))
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
                        self->_event_handler(self, HIO_CMWX1ZZABZ_EVENT_JOIN_SUCCESS, self->_event_param);
                    }
                }
                else
                {
                    if (self->_event_handler != NULL)
                    {
                        self->_event_handler(self, HIO_CMWX1ZZABZ_EVENT_JOIN_ERROR, self->_event_param);
                    }
                }

                self->_state = HIO_CMWX1ZZABZ_STATE_IDLE;
                continue;
            }

            default:
            {
                break;
            }
        }
    }
}

void hio_cmwx1zzabz_join(hio_cmwx1zzabz_t *self)
{
    self->_join_command = true;
    hio_scheduler_plan_now(self->_task_id);
}

void hio_cmwx1zzabz_set_port(hio_cmwx1zzabz_t *self, uint8_t port)
{
    self->_tx_port = port;
}

uint8_t hio_cmwx1zzabz_get_port(hio_cmwx1zzabz_t *self)
{
    return self->_tx_port;
}

void hio_cmwx1zzabz_set_devaddr(hio_cmwx1zzabz_t *self, char *devaddr)
{
    strncpy(self->_config.devaddr, devaddr, 8+1);

    _hio_cmwx1zzabz_save_config(self, HIO_CMWX1ZZABZ_CONFIG_INDEX_DEVADDR);
}

void hio_cmwx1zzabz_get_devaddr(hio_cmwx1zzabz_t *self, char *devaddr)
{
    strncpy(devaddr, self->_config.devaddr, 8+1);
}

void hio_cmwx1zzabz_set_deveui(hio_cmwx1zzabz_t *self, char *deveui)
{
    strncpy(self->_config.deveui, deveui, 16+1);

    _hio_cmwx1zzabz_save_config(self, HIO_CMWX1ZZABZ_CONFIG_INDEX_DEVEUI);
}

void hio_cmwx1zzabz_get_deveui(hio_cmwx1zzabz_t *self, char *deveui)
{
    strncpy(deveui, self->_config.deveui, 16+1);
}

void hio_cmwx1zzabz_set_appeui(hio_cmwx1zzabz_t *self, char *appeui)
{
    strncpy(self->_config.appeui, appeui, 16+1);

    _hio_cmwx1zzabz_save_config(self, HIO_CMWX1ZZABZ_CONFIG_INDEX_APPEUI);
}

void hio_cmwx1zzabz_get_appeui(hio_cmwx1zzabz_t *self, char *appeui)
{
    strncpy(appeui, self->_config.appeui, 16+1);
}

void hio_cmwx1zzabz_set_nwkskey(hio_cmwx1zzabz_t *self, char *nwkskey)
{
    strncpy(self->_config.nwkskey, nwkskey, 32);

    _hio_cmwx1zzabz_save_config(self, HIO_CMWX1ZZABZ_CONFIG_INDEX_NWKSKEY);
}

void hio_cmwx1zzabz_get_nwkskey(hio_cmwx1zzabz_t *self, char *nwkskey)
{
    strncpy(nwkskey, self->_config.nwkskey, 32+1);
}

void hio_cmwx1zzabz_set_appskey(hio_cmwx1zzabz_t *self, char *appskey)
{
    strncpy(self->_config.appskey, appskey, 32);

    _hio_cmwx1zzabz_save_config(self, HIO_CMWX1ZZABZ_CONFIG_INDEX_APPSKEY);
}

void hio_cmwx1zzabz_get_appskey(hio_cmwx1zzabz_t *self, char *appskey)
{
    strncpy(appskey, self->_config.appskey, 32+1);
}

void hio_cmwx1zzabz_set_appkey(hio_cmwx1zzabz_t *self, char *appkey)
{
    strncpy(self->_config.appkey, appkey, 32+1);

    _hio_cmwx1zzabz_save_config(self, HIO_CMWX1ZZABZ_CONFIG_INDEX_APPKEY);
}

void hio_cmwx1zzabz_get_appkey(hio_cmwx1zzabz_t *self, char *appkey)
{
    strncpy(appkey, self->_config.appkey, 32+1);
}

void hio_cmwx1zzabz_set_band(hio_cmwx1zzabz_t *self, hio_cmwx1zzabz_config_band_t band)
{
    self->_config.band = band;

    _hio_cmwx1zzabz_save_config(self, HIO_CMWX1ZZABZ_CONFIG_INDEX_BAND);
}

hio_cmwx1zzabz_config_band_t hio_cmwx1zzabz_get_band(hio_cmwx1zzabz_t *self)
{
    return self->_config.band;
}

void hio_cmwx1zzabz_set_mode(hio_cmwx1zzabz_t *self, hio_cmwx1zzabz_config_mode_t mode)
{
    self->_config.mode = mode;

    _hio_cmwx1zzabz_save_config(self, HIO_CMWX1ZZABZ_CONFIG_INDEX_MODE);
}

hio_cmwx1zzabz_config_mode_t hio_cmwx1zzabz_get_mode(hio_cmwx1zzabz_t *self)
{
    return self->_config.mode;
}

void hio_cmwx1zzabz_set_class(hio_cmwx1zzabz_t *self, hio_cmwx1zzabz_config_class_t class)
{
    self->_config.class = class;

    _hio_cmwx1zzabz_save_config(self, HIO_CMWX1ZZABZ_CONFIG_INDEX_CLASS);
}

hio_cmwx1zzabz_config_class_t hio_cmwx1zzabz_get_class(hio_cmwx1zzabz_t *self)
{
    return self->_config.class;
}

uint8_t hio_cmwx1zzabz_get_received_message_port(hio_cmwx1zzabz_t *self)
{
    return self->_message_port;
}

uint32_t hio_cmwx1zzabz_get_received_message_length(hio_cmwx1zzabz_t *self)
{
    return self->_message_length;
}

uint32_t hio_cmwx1zzabz_get_received_message_data(hio_cmwx1zzabz_t *self, uint8_t *buffer, uint32_t buffer_size)
{
    if (self->_message_length > buffer_size)
    {
        return 0;
    }

    memcpy(buffer, self->_message_buffer, self->_message_length);

    return self->_message_length;
}

void hio_cmwx1zzabz_set_rx2(hio_cmwx1zzabz_t *self, uint32_t frequency, uint8_t datarate)
{
    self->_config.rx2_frequency = frequency;

    self->_config.rx2_datarate = datarate;

    _hio_cmwx1zzabz_save_config(self, HIO_CMWX1ZZABZ_CONFIG_INDEX_RX2);
}

void hio_cmwx1zzabz_get_rx2(hio_cmwx1zzabz_t *self, uint32_t *frequency, uint8_t *datarate)
{
    *frequency = self->_config.rx2_frequency;
    *datarate = self->_config.rx2_datarate;
}

void hio_cmwx1zzabz_set_nwk_public(hio_cmwx1zzabz_t *self, uint8_t public)
{
    self->_config.nwk_public = public;

    _hio_cmwx1zzabz_save_config(self, HIO_CMWX1ZZABZ_CONFIG_INDEX_NWK);
}

uint8_t hio_cmwx1zzabz_get_nwk_public(hio_cmwx1zzabz_t *self)
{
    return self->_config.nwk_public;
}

void hio_cmwx1zzabz_set_datarate(hio_cmwx1zzabz_t *self, uint8_t datarate)
{
    self->_config.datarate = datarate;

    _hio_cmwx1zzabz_save_config(self, HIO_CMWX1ZZABZ_CONFIG_INDEX_DATARATE);
}

uint8_t hio_cmwx1zzabz_get_datarate(hio_cmwx1zzabz_t *self)
{
    return self->_config.datarate;
}

char *hio_cmwx1zzabz_get_error_command(hio_cmwx1zzabz_t *self)
{
    return self->_command;
}

char *hio_cmwx1zzabz_get_error_response(hio_cmwx1zzabz_t *self)
{
    return self->_response;
}

static bool _hio_cmwx1zzabz_read_response(hio_cmwx1zzabz_t *self)
{
    size_t length = 0;

    while (true)
    {
        char rx_character;

        if (hio_uart_async_read(self->_uart_channel, &rx_character, 1) == 0)
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

    if (self->_debug)
    {
        hio_log_debug("LoRa RX: %s", (const char*)self->_response);
    }

    return true;
}

static void _hio_cmwx1zzabz_save_config(hio_cmwx1zzabz_t *self, hio_cmwx1zzabz_config_index_t config_index)
{
    self->_save_config_mask |= 1 << config_index;

    if (self->_state == HIO_CMWX1ZZABZ_STATE_IDLE)
    {
        hio_scheduler_plan_now(self->_task_id);
    }
}
