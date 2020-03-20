#include <bc_esp8266.h>
#include <bc_rtc.h>

#define _BC_ESP8266_DELAY_INITIALIZATION_AT_COMMAND 100
#define _BC_ESP8266_DELAY_SEND_RESPONSE 100
#define _BC_ESP8266_DELAY_WIFI_CONNECT 1000
#define _BC_ESP8266_DELAY_SOCKET_CONNECT 300
#define _BC_ESP8266_TIMEOUT_WIFI_CONNECT 20
#define _BC_ESP8266_TIMEOUT_SOCKET_CONNECT 10

// Apply changes to the factory configuration
static const char *_esp8266_init_commands[] =
{
    // Disable AT Commands Echoing
    "ATE0\r\n",
    // Set Station mode
    "AT+CWMODE=1\r\n",
    // Disable Multiple Connections
    "AT+CIPMUX=0\r\n",
    // Does not Show the Remote IP and Port with +IPD
    "AT+CIPDINFO=0\r\n",
    // Disable Auto-Connect
    "AT+CWAUTOCONN=0\r\n",
    // Enable DHCP
    "AT+CWDHCP=1,1\r\n",
    // Sets the Size of SSL Buffer
    "AT+CIPSSLSIZE=4096\r\n",
    NULL
};

static void _bc_esp8266_task(void *param);
static bool _bc_esp8266_read_response(bc_esp8266_t *self);
static bool _bc_esp8266_read_socket_data(bc_esp8266_t *self);
static void _uart_event_handler(bc_uart_channel_t channel, bc_uart_event_t event, void *param);
static void _bc_esp8266_set_rtc_time(char *str);

void bc_esp8266_init(bc_esp8266_t *self,  bc_uart_channel_t uart_channel)
{
    memset(self, 0, sizeof(*self));

    self->_uart_channel = uart_channel;

    self->_config.mode = BC_ESP8266_CONFIG_MODE_STATION;
    self->_config.ssid[0] = '\0';
    self->_config.password[0] = '\0';
    self->_config.sntp_enabled = 0;

    // CH_PD of ESP8266
    bc_gpio_init(BC_GPIO_P8);
    bc_gpio_set_mode(BC_GPIO_P8, BC_GPIO_MODE_OUTPUT);
    bc_gpio_set_output(BC_GPIO_P8, 0);

    // RESET of ESP8266
    bc_gpio_init(BC_GPIO_P6);
    bc_gpio_set_mode(BC_GPIO_P6, BC_GPIO_MODE_OUTPUT);
    bc_gpio_set_output(BC_GPIO_P6, 1);

    bc_fifo_init(&self->_tx_fifo, self->_tx_fifo_buffer, sizeof(self->_tx_fifo_buffer));
    bc_fifo_init(&self->_rx_fifo, self->_rx_fifo_buffer, sizeof(self->_rx_fifo_buffer));

    self->_task_id = bc_scheduler_register(_bc_esp8266_task, self, BC_TICK_INFINITY);
    self->_state = BC_ESP8266_STATE_DISCONNECTED;
}

static void _uart_event_handler(bc_uart_channel_t channel, bc_uart_event_t event, void *param)
{
    (void) channel;
    bc_esp8266_t *self = (bc_esp8266_t*)param;

    if (event == BC_UART_EVENT_ASYNC_READ_DATA && self->_state == BC_ESP8266_STATE_IDLE)
    {
        bc_scheduler_plan_relative(self->_task_id, 100);
        self->_state = BC_ESP8266_STATE_RECEIVE;
    }
}

void _bc_esp8266_enable(bc_esp8266_t *self)
{
    // Initialize UART
    bc_uart_init(self->_uart_channel, BC_UART_BAUDRATE_115200, BC_UART_SETTING_8N1);
    bc_uart_set_async_fifo(self->_uart_channel, &self->_tx_fifo, &self->_rx_fifo);
    bc_uart_async_read_start(self->_uart_channel, BC_TICK_INFINITY);
    bc_uart_set_event_handler(self->_uart_channel, _uart_event_handler, self);

    // Enable CH_PD
    bc_gpio_set_output(BC_GPIO_P8, 1);
}

void _bc_esp8266_disable(bc_esp8266_t *self)
{
    // Disable CH_PD
    bc_gpio_set_output(BC_GPIO_P8, 0);

    // Deinitialize UART
    bc_uart_deinit(self->_uart_channel);
}

void bc_esp8266_set_event_handler(bc_esp8266_t *self, void (*event_handler)(bc_esp8266_t *, bc_esp8266_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_esp8266_set_station_mode(bc_esp8266_t *self, char *ssid, char *password)
{
    self->_config.mode = BC_ESP8266_CONFIG_MODE_STATION;
    strncpy(self->_config.ssid, ssid, 63);
    strncpy(self->_config.password, password, 63);
}

void bc_esp8266_set_sntp(bc_esp8266_t *self, int timezone)
{
    bc_esp8266_set_sntp_with_servers(self, timezone, NULL, NULL, NULL);
}

void bc_esp8266_set_sntp_with_servers(bc_esp8266_t *self, int timezone, char *sntp_server1, char *sntp_server2, char *sntp_server3)
{
    self->_config.sntp_enabled = 1;
    self->_config.sntp_timezone = timezone;
    if (sntp_server1 != NULL)
    {
        strncpy(self->_config.sntp_server1, sntp_server1, 127);
    }
    else
    {
        strcpy(self->_config.sntp_server1, "0.pool.ntp.org");
    }
    if (sntp_server2 != NULL)
    {
        strncpy(self->_config.sntp_server2, sntp_server2, 127);
    }
    else
    {
        strcpy(self->_config.sntp_server2, "1.pool.ntp.org");
    }
    if (sntp_server3 != NULL)
    {
        strncpy(self->_config.sntp_server3, sntp_server3, 127);
    }
    else
    {
        strcpy(self->_config.sntp_server3, "2.pool.ntp.org");
    }
}

bool bc_esp8266_is_ready(bc_esp8266_t *self)
{
    if (self->_state == BC_ESP8266_STATE_READY || self->_state == BC_ESP8266_STATE_IDLE)
    {
        return true;
    }

    return false;
}

bool bc_esp8266_connect(bc_esp8266_t *self)
{
    if (self->_state != BC_ESP8266_STATE_DISCONNECTED ||
        self->_config.ssid[0] == '\0' || self->_config.password[0] == '\0')
    {
        return false;
    }

    _bc_esp8266_enable(self);

    self->_state = BC_ESP8266_STATE_INITIALIZE;
    self->_state_after_init = BC_ESP8266_STATE_WIFI_CONNECT_COMMAND;

    bc_scheduler_plan_now(self->_task_id);

    return true;
}

bool bc_esp8266_disconnect(bc_esp8266_t *self)
{
    _bc_esp8266_disable(self);

    self->_state = BC_ESP8266_STATE_DISCONNECTED;

    if (self->_event_handler != NULL)
    {
        self->_event_handler(self, BC_ESP8266_EVENT_DISCONNECTED, self->_event_param);
    }

    return true;
}

bool bc_esp8266_socket_connect(bc_esp8266_t *self, const char *type, const char *host, uint16_t port)
{
    if (!bc_esp8266_is_ready(self) || host == NULL || port == 0 || strlen(host) == 0 ||
        (strlen(host) + 15) > BC_ESP8266_TX_MAX_PACKET_SIZE)
    {
        return false;
    }

    static char buffer[BC_ESP8266_TX_MAX_PACKET_SIZE];
    sprintf(buffer, "\"%s\",\"%s\",%d", type, host, port);

    self->_message_length = strlen(buffer);

    memcpy(self->_message_buffer, buffer, self->_message_length);

    self->_state = BC_ESP8266_STATE_SOCKET_CONNECT_COMMAND;

    bc_scheduler_plan_now(self->_task_id);

    return true;
}

bool bc_esp8266_tcp_connect(bc_esp8266_t *self, const char *host, uint16_t port)
{
    return bc_esp8266_socket_connect(self, "TCP", host, port);
}

bool bc_esp8266_udp_connect(bc_esp8266_t *self, const char *host, uint16_t port)
{
    return bc_esp8266_socket_connect(self, "UDP", host, port);
}

bool bc_esp8266_ssl_connect(bc_esp8266_t *self, const char *host, uint16_t port)
{
    return bc_esp8266_socket_connect(self, "SSL", host, port);
}

bool bc_esp8266_send_data(bc_esp8266_t *self, const void *buffer, size_t length)
{
    if (!bc_esp8266_is_ready(self) || length == 0 || length > BC_ESP8266_TX_MAX_PACKET_SIZE)
    {
        return false;
    }

    self->_message_length = length;

    memcpy(self->_message_buffer, buffer, self->_message_length);

    self->_state = BC_ESP8266_STATE_SOCKET_SEND_COMMAND;

    bc_scheduler_plan_now(self->_task_id);

    return true;
}

static void _bc_esp8266_task(void *param)
{
    bc_esp8266_t *self = param;

    while (true)
    {
        switch (self->_state)
        {
            case BC_ESP8266_STATE_READY:
            {
                self->_state = BC_ESP8266_STATE_IDLE;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_ESP8266_EVENT_READY, self->_event_param);
                }

                continue;
            }
            case BC_ESP8266_STATE_IDLE:
            case BC_ESP8266_STATE_DISCONNECTED:
            {
                return;
            }
            case BC_ESP8266_STATE_RECEIVE:
            {
                self->_state = BC_ESP8266_STATE_IDLE;

                while (_bc_esp8266_read_response(self))
                {
                    if (memcmp(self->_response, "+IPD", 4) == 0)
                    {
                        // Data length is between "," and ":"
                        char *comma_search = strchr(self->_response, ',');
                        if (comma_search == NULL)
                        {
                            continue;
                        }
                        comma_search++;
                        char *colon_search = strchr(self->_response, ':');
                        if (colon_search == NULL)
                        {
                            continue;
                        }
                        if ((colon_search - comma_search) > 9)
                        {
                            continue;
                        }
                        char length_text[10];
                        memcpy(length_text, comma_search, colon_search - comma_search);
                        length_text[colon_search - comma_search] = '\0';
                        self->_message_length = atoi(length_text);

                        self->_message_part_length = (strlen(self->_response) - 1) - (colon_search - self->_response);
                        colon_search++;
                        memcpy(self->_message_buffer, colon_search, self->_message_part_length);

                        if (self->_message_length > sizeof(self->_message_buffer))
                        {
                            self->_message_length = sizeof(self->_message_buffer);
                        }

                        self->_state = BC_ESP8266_STATE_SOCKET_RECEIVE;

                        bc_scheduler_plan_current_now();
                        return;
                    }
                }

                return;
            }
            case BC_ESP8266_STATE_ERROR:
            {
                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_ESP8266_EVENT_ERROR, self->_event_param);
                }

                self->_state = BC_ESP8266_STATE_INITIALIZE;

                continue;
            }
            case BC_ESP8266_STATE_INITIALIZE:
            {
                self->_init_command_index = 0;
                self->_state = BC_ESP8266_STATE_INITIALIZE_COMMAND_SEND;

                continue;
            }
            case BC_ESP8266_STATE_INITIALIZE_COMMAND_SEND:
            {
                self->_state = BC_ESP8266_STATE_ERROR;

                // Purge RX FIFO
                char rx_character;
                while (bc_uart_async_read(self->_uart_channel, &rx_character, 1) != 0)
                {
                }

                strcpy(self->_command, _esp8266_init_commands[self->_init_command_index]);
                size_t length = strlen(self->_command);

                if (bc_uart_async_write(self->_uart_channel, self->_command, length) != length)
                {
                    continue;
                }

                self->_state = BC_ESP8266_STATE_INITIALIZE_COMMAND_RESPONSE;
                bc_scheduler_plan_current_from_now(_BC_ESP8266_DELAY_INITIALIZATION_AT_COMMAND);

                return;
            }
            case BC_ESP8266_STATE_INITIALIZE_COMMAND_RESPONSE:
            {
                self->_state = BC_ESP8266_STATE_ERROR;

                if (!_bc_esp8266_read_response(self))
                {
                    continue;
                }

                // repeated command, continue reading response
                if (memcmp(self->_response, "AT", 2) == 0 && !_bc_esp8266_read_response(self))
                {
                    continue;
                }

                if (memcmp(self->_response, "OK", 2) != 0)
                {
                    continue;
                }

                self->_init_command_index++;

                if (_esp8266_init_commands[self->_init_command_index] == NULL)
                {
                    self->_state = self->_state_after_init;
                }
                else
                {
                    self->_state = BC_ESP8266_STATE_INITIALIZE_COMMAND_SEND;
                }

                continue;
            }
            case BC_ESP8266_STATE_WIFI_CONNECT_COMMAND:
            case BC_ESP8266_STATE_AP_AVAILABILITY_OPT_COMMAND:
            case BC_ESP8266_STATE_AP_AVAILABILITY_COMMAND:
            case BC_ESP8266_STATE_SNTP_CONFIG_COMMAND:
            case BC_ESP8266_STATE_SNTP_TIME_COMMAND:
            case BC_ESP8266_STATE_SOCKET_CONNECT_COMMAND:
            case BC_ESP8266_STATE_SOCKET_SEND_COMMAND:
            {
                bc_esp8266_state_t response_state;

                if (self->_state == BC_ESP8266_STATE_WIFI_CONNECT_COMMAND)
                {
                    sprintf(self->_command, "AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", self->_config.ssid, self->_config.password);
                    response_state = BC_ESP8266_STATE_WIFI_CONNECT_RESPONSE;
                }
                else if (self->_state == BC_ESP8266_STATE_AP_AVAILABILITY_OPT_COMMAND)
                {
                    strcpy(self->_command, "AT+CWLAPOPT=1,6\r\n");
                    response_state = BC_ESP8266_STATE_AP_AVAILABILITY_OPT_RESPONSE;
                }
                else if (self->_state == BC_ESP8266_STATE_AP_AVAILABILITY_COMMAND)
                {
                    strcpy(self->_command, "AT+CWLAP\r\n");
                    self->_rssi = 0;
                    self->_ap_available = false;
                    response_state = BC_ESP8266_STATE_AP_AVAILABILITY_RESPONSE;
                }
                else if (self->_state == BC_ESP8266_STATE_SNTP_CONFIG_COMMAND)
                {
                    sprintf(self->_command, "AT+CIPSNTPCFG=%u,%d,\"%s\",\"%s\",\"%s\"\r\n",
                        self->_config.sntp_enabled,
                        self->_config.sntp_timezone,
                        self->_config.sntp_server1,
                        self->_config.sntp_server2,
                        self->_config.sntp_server3);
                    response_state = BC_ESP8266_STATE_SNTP_CONFIG_RESPONSE;
                }
                else if (self->_state == BC_ESP8266_STATE_SNTP_TIME_COMMAND)
                {
                    strcpy(self->_command, "AT+CIPSNTPTIME?\r\n");
                    response_state = BC_ESP8266_STATE_SNTP_TIME_RESPONSE;
                }
                else if (self->_state == BC_ESP8266_STATE_SOCKET_CONNECT_COMMAND)
                {
                    strcpy(self->_command, "AT+CIPSTART=");
                    response_state = BC_ESP8266_STATE_SOCKET_CONNECT_RESPONSE;
                }
                else
                {
                    sprintf(self->_command, "AT+CIPSEND=%d\r\n", self->_message_length);
                    response_state = BC_ESP8266_STATE_SOCKET_SEND_DATA;
                }

                self->_state = BC_ESP8266_STATE_ERROR;

                uint8_t command_length = strlen(self->_command);
                size_t length = command_length;

                if (response_state == BC_ESP8266_STATE_SOCKET_CONNECT_RESPONSE)
                {
                    for (size_t i = 0; i < self->_message_length; i++)
                    {
                        // put binary data directly to the "string" buffer
                        self->_command[command_length + i] = self->_message_buffer[i];
                    }
                    self->_command[command_length + self->_message_length] = '\r';
                    self->_command[command_length + self->_message_length + 1] = '\n';

                    length = command_length + self->_message_length + 2;
                }

                if (bc_uart_async_write(self->_uart_channel, self->_command, length) != length)
                {
                    continue;
                }

                self->_state = response_state;
                self->_timeout_cnt = 0;

                bc_scheduler_plan_current_from_now(_BC_ESP8266_DELAY_SEND_RESPONSE);

                return;
            }
            case BC_ESP8266_STATE_SOCKET_SEND_DATA:
            {
                self->_state = BC_ESP8266_STATE_ERROR;

                if (bc_uart_async_write(self->_uart_channel, self->_message_buffer, self->_message_length) != self->_message_length)
                {
                    continue;
                }

                self->_state = BC_ESP8266_STATE_SOCKET_SEND_RESPONSE;

                bc_scheduler_plan_current_from_now(_BC_ESP8266_DELAY_SEND_RESPONSE);

                return;
            }
            case BC_ESP8266_STATE_WIFI_CONNECT_RESPONSE:
            {
                /*
                Success response:
                WIFI DISCONNECT
                WIFI CONNECTED
                WIFI GOT IP

                OK
                */

                self->_timeout_cnt++;
                if (self->_timeout_cnt > _BC_ESP8266_TIMEOUT_WIFI_CONNECT)
                {
                    self->_state = BC_ESP8266_STATE_WIFI_CONNECT_ERROR;
                    continue;
                }

                if (!_bc_esp8266_read_response(self))
                {
                    bc_scheduler_plan_current_from_now(_BC_ESP8266_DELAY_WIFI_CONNECT);
                    return;
                }

                if (strcmp(self->_response, "OK\r") == 0)
                {
                    if (self->_config.sntp_enabled)
                    {
                        self->_state = BC_ESP8266_STATE_SNTP_CONFIG_COMMAND;
                    }
                    else
                    {
                        self->_state = BC_ESP8266_STATE_READY;
                        if (self->_event_handler != NULL)
                        {
                            self->_event_handler(self, BC_ESP8266_EVENT_WIFI_CONNECT_SUCCESS, self->_event_param);
                        }
                    }
                }
                else if (strcmp(self->_response, "FAIL\r") == 0)
                {
                    self->_state = BC_ESP8266_STATE_WIFI_CONNECT_ERROR;
                }
                else
                {
                    bc_scheduler_plan_current_from_now(_BC_ESP8266_DELAY_WIFI_CONNECT);
                    return;
                }

                continue;
            }
            case BC_ESP8266_STATE_WIFI_CONNECT_ERROR:
            {
                self->_state = BC_ESP8266_STATE_DISCONNECTED;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_ESP8266_EVENT_WIFI_CONNECT_ERROR, self->_event_param);
                }

                continue;
            }
            case BC_ESP8266_STATE_SNTP_CONFIG_RESPONSE:
            {
                self->_state = BC_ESP8266_STATE_ERROR;

                if (!_bc_esp8266_read_response(self))
                {
                    continue;
                }

                if (memcmp(self->_response, "OK", 2) != 0)
                {
                    continue;
                }

                self->_state = BC_ESP8266_STATE_SNTP_TIME_COMMAND;

                continue;
            }
            case BC_ESP8266_STATE_SNTP_TIME_RESPONSE:
            {
                /*
                Success response:
                +CIPSNTPTIME:Sun Jul 21 12:02:06 2019
                OK
                */

                self->_timeout_cnt++;
                if (self->_timeout_cnt > _BC_ESP8266_TIMEOUT_SOCKET_CONNECT)
                {
                    self->_state = BC_ESP8266_STATE_WIFI_CONNECT_ERROR;
                    continue;
                }

                if (!_bc_esp8266_read_response(self))
                {
                    bc_scheduler_plan_current_from_now(_BC_ESP8266_DELAY_SOCKET_CONNECT);
                    return;
                }

                if (strcmp(self->_response, "OK\r") == 0)
                {
                    self->_state = BC_ESP8266_STATE_READY;
                    if (self->_event_handler != NULL)
                    {
                        self->_event_handler(self, BC_ESP8266_EVENT_WIFI_CONNECT_SUCCESS, self->_event_param);
                    }
                }
                else if (memcmp(self->_response, "+CIPSNTPTIME:", 13) == 0)
                {
                    _bc_esp8266_set_rtc_time(self->_response + 13);
                }
                else
                {
                    bc_scheduler_plan_current_from_now(_BC_ESP8266_DELAY_SOCKET_CONNECT);
                    return;
                }

                continue;
            }
            case BC_ESP8266_STATE_SOCKET_CONNECT_RESPONSE:
            {
                /*
                Success response:
                CONNECT

                OK
                */

                self->_timeout_cnt++;
                if (self->_timeout_cnt > _BC_ESP8266_TIMEOUT_SOCKET_CONNECT)
                {
                    self->_state = BC_ESP8266_STATE_SOCKET_CONNECT_ERROR;
                    continue;
                }

                if (!_bc_esp8266_read_response(self))
                {
                    bc_scheduler_plan_current_from_now(_BC_ESP8266_DELAY_SOCKET_CONNECT);
                    return;
                }

                if (strcmp(self->_response, "OK\r") == 0)
                {
                    self->_state = BC_ESP8266_STATE_READY;
                    if (self->_event_handler != NULL)
                    {
                        self->_event_handler(self, BC_ESP8266_EVENT_SOCKET_CONNECT_SUCCESS, self->_event_param);
                    }
                }
                else if (strcmp(self->_response, "ERROR\r") == 0)
                {
                    self->_state = BC_ESP8266_STATE_SOCKET_CONNECT_ERROR;
                }
                else
                {
                    bc_scheduler_plan_current_from_now(_BC_ESP8266_DELAY_SOCKET_CONNECT);
                    return;
                }

                continue;
            }
            case BC_ESP8266_STATE_SOCKET_CONNECT_ERROR:
            {
                self->_state = BC_ESP8266_STATE_READY;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_ESP8266_EVENT_SOCKET_CONNECT_ERROR, self->_event_param);
                }

                continue;
            }
            case BC_ESP8266_STATE_SOCKET_SEND_RESPONSE:
            {
                self->_state = BC_ESP8266_STATE_SOCKET_SEND_ERROR;

                if (!_bc_esp8266_read_response(self))
                {
                    continue;
                }

                if (strcmp(self->_response, "OK\r") != 0)
                {
                    continue;
                }

                self->_state = BC_ESP8266_STATE_RECEIVE;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_ESP8266_EVENT_SOCKET_SEND_SUCCESS, self->_event_param);
                }

                continue;
            }
            case BC_ESP8266_STATE_SOCKET_SEND_ERROR:
            {
                self->_state = BC_ESP8266_STATE_READY;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_ESP8266_EVENT_SOCKET_SEND_ERROR, self->_event_param);
                }

                continue;
            }
            case BC_ESP8266_STATE_SOCKET_RECEIVE:
            {
                if (!_bc_esp8266_read_socket_data(self))
                {
                    continue;
                }

                self->_state = BC_ESP8266_STATE_READY;

                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_ESP8266_EVENT_DATA_RECEIVED, self->_event_param);
                }

                continue;
            }
            case BC_ESP8266_STATE_AP_AVAILABILITY_OPT_RESPONSE:
            {
                if (!_bc_esp8266_read_response(self) || memcmp(self->_response, "OK", 2) != 0)
                {
                    bc_esp8266_disconnect(self);
                    return;
                }

                self->_state = BC_ESP8266_STATE_AP_AVAILABILITY_COMMAND;

                continue;
            }
            case BC_ESP8266_STATE_AP_AVAILABILITY_RESPONSE:
            {
                /*
                Success response:
                +CWLAP:("Internet_7E",-74)
                +CWLAP:("WLAN1-R87LDH",-77)
                OK
                */

                self->_timeout_cnt++;
                if (self->_timeout_cnt > _BC_ESP8266_TIMEOUT_WIFI_CONNECT)
                {
                    bc_esp8266_disconnect(self);
                    return;
                }

                if (!_bc_esp8266_read_response(self))
                {
                    bc_scheduler_plan_current_from_now(_BC_ESP8266_DELAY_WIFI_CONNECT);
                    return;
                }

                if (strcmp(self->_response, "OK\r") == 0)
                {
                    bc_esp8266_disconnect(self);

                    if (self->_event_handler != NULL)
                    {
                        self->_event_handler(self, BC_ESP8266_EVENT_AP_AVAILABILITY_RESULT, self->_event_param);
                    }
                    return;
                }
                else if (strcmp(self->_response, "ERROR\r") == 0)
                {
                    bc_esp8266_disconnect(self);
                    return;
                }
                else
                {
                    char text[76];
                    sprintf(text, "+CWLAP:(\"%s\",", self->_config.ssid);
                    size_t text_len = strlen(text);
                    if (strncmp(self->_response, text, text_len) == 0)
                    {
                        char *rssi = self->_response + text_len;
                        char *end = strchr(rssi, ')');
                        if (end != NULL)
                        {
                            end[0] = '\0';
                            self->_rssi = atoi(rssi);
                            self->_ap_available = true;
                        }
                    }

                    bc_scheduler_plan_current_from_now(_BC_ESP8266_DELAY_WIFI_CONNECT);
                    return;
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

uint32_t bc_esp8266_get_received_message_length(bc_esp8266_t *self)
{
    return self->_message_length;
}

uint32_t bc_esp8266_get_received_message_data(bc_esp8266_t *self, uint8_t *buffer, uint32_t buffer_size)
{
    if (self->_message_length > buffer_size)
    {
        return 0;
    }

    memcpy(buffer, self->_message_buffer, self->_message_length);

    return self->_message_length;
}

static bool _bc_esp8266_read_response(bc_esp8266_t *self)
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

static bool _bc_esp8266_read_socket_data(bc_esp8266_t *self)
{
    while (true)
    {
        char rx_character;

        if (bc_uart_async_read(self->_uart_channel, &rx_character, 1) == 0)
        {
            return false;
        }

        self->_message_buffer[self->_message_part_length++] = rx_character;

        if (self->_message_part_length == self->_message_length)
        {
            break;
        }
    }

    return true;
}

static void _bc_esp8266_set_rtc_time(char *str)
{
    bc_rtc_t rtc;
    char token[5];
    uint8_t j = 0;
    uint8_t state = 0;
    size_t length = strlen(str);
    for (size_t i = 0; i <= length; i++)
    {
        char c = str[i];
        if (c == ' ' || c == ':' || c == '\0' || j == 4)
        {
            token[j] = '\0';

            // Month
            if (state == 1)
            {
                if (strcmp(token, "Jan") == 0)
                {
                    rtc.month = 1;
                }
                else if (strcmp(token, "Feb") == 0)
                {
                    rtc.month = 2;
                }
                else if (strcmp(token, "Mar") == 0)
                {
                    rtc.month = 3;
                }
                else if (strcmp(token, "Apr") == 0)
                {
                    rtc.month = 4;
                }
                else if (strcmp(token, "May") == 0)
                {
                    rtc.month = 5;
                }
                else if (strcmp(token, "Jun") == 0)
                {
                    rtc.month = 6;
                }
                else if (strcmp(token, "Jul") == 0)
                {
                    rtc.month = 7;
                }
                else if (strcmp(token, "Aug") == 0)
                {
                    rtc.month = 8;
                }
                else if (strcmp(token, "Sep") == 0)
                {
                    rtc.month = 9;
                }
                else if (strcmp(token, "Oct") == 0)
                {
                    rtc.month = 10;
                }
                else if (strcmp(token, "Nov") == 0)
                {
                    rtc.month = 11;
                }
                else if (strcmp(token, "Dec") == 0)
                {
                    rtc.month = 12;
                }
            }
            // Day
            else if (state == 2)
            {
                rtc.date = atoi(token);
            }
            // Hours
            else if (state == 3)
            {
                rtc.hours = atoi(token);
            }
            // Minutes
            else if (state == 4)
            {
                rtc.minutes = atoi(token);
            }
            // Seconds
            else if (state == 5)
            {
                rtc.seconds = atoi(token);
            }
            // Year
            else if (state == 6)
            {
                if (strcmp(token, "1970") == 0)
                {
                    return;
                }
                rtc.year = atoi(token);
            }

            j = 0;
            state++;
        }
        else
        {
            token[j++] = c;
        }
    }
    bc_rtc_set_date_time(&rtc);
}

bool bc_esp8266_check_ap_availability(bc_esp8266_t *self)
{
    if (self->_state != BC_ESP8266_STATE_DISCONNECTED || self->_config.ssid[0] == '\0')
    {
        return false;
    }

    _bc_esp8266_enable(self);

    self->_state = BC_ESP8266_STATE_INITIALIZE;
    self->_state_after_init = BC_ESP8266_STATE_AP_AVAILABILITY_OPT_COMMAND;

    bc_scheduler_plan_now(self->_task_id);

    return true;
}

void bc_esp8266_get_ap_availability(bc_esp8266_t *self, bool *available, int *rssi)
{
    *available = self->_ap_available;
    *rssi = self->_rssi;
}

void bc_esp8266_get_ssid(bc_esp8266_t *self, char *ssid)
{
    strncpy(ssid, self->_config.ssid, 64);
}

void bc_esp8266_set_ssid(bc_esp8266_t *self, char *ssid)
{
    strncpy(self->_config.ssid, ssid, 64);
}

void bc_esp8266_get_password(bc_esp8266_t *self, char *password)
{
    strncpy(password, self->_config.password, 64);
}

void bc_esp8266_set_password(bc_esp8266_t *self, char *password)
{
    strncpy(self->_config.password, password, 64);
}
