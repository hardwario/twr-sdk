#ifndef _TWR_ATCI_H
#define _TWR_ATCI_H

#include <twr_uart.h>

//! @addtogroup twr_atci twr_atci
//! @brief AT command interface
//! @{

#ifndef TWR_ATCI_UART
#define TWR_ATCI_UART TWR_UART_UART2
#endif

#define TWR_ATCI_COMMANDS_LENGTH(COMMANDS) (sizeof(COMMANDS) / sizeof(COMMANDS[0]))

#define TWR_ATCI_COMMAND_CLAC {"+CLAC", twr_atci_clac_action, NULL, NULL, NULL, "List all available AT commands"}
#define TWR_ATCI_COMMAND_HELP {"$HELP", twr_atci_help_action, NULL, NULL, NULL, "This help"}

typedef struct
{
    char *txt;
    size_t length;
    size_t offset;

} twr_atci_param_t;

//! @brief AT command struct

typedef struct
{
  const char *command;
  bool (*action)(void);
  bool (*set)(twr_atci_param_t *param);
  bool (*read)(void);
  bool (*help)(void);
  const char *hint;

} twr_atci_command_t;

//! @brief Initialize
//! @param[in] commands
//! @param[in] length Number of commands

void twr_atci_init(const twr_atci_command_t *commands, int length);

//! @brief Write OK

void twr_atci_write_ok(void);

//! @brief Write ERROR

void twr_atci_write_error(void);

//! @brief Print message
//! @param[in] message Message
//! @return Number of bytes written

size_t twr_atci_print(const char *message);

//! @brief Print message and add CR LF
//! @param[in] message Message
//! @return Number of bytes written

size_t twr_atci_println(const char *message);

//! @brief Prinf message
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments
//! @return Number of bytes written

size_t twr_atci_printf(const char *format, ...);

//! @brief Prinf message and add CR LF
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments
//! @return Number of bytes written

size_t twr_atci_printfln(const char *format, ...);

//! @brief Print buffer as HEX string
//! @param[in] buffer Pointer to source buffer
//! @param[in] length Number of bytes to be written
//! @return Number of bytes written

size_t twr_atci_print_buffer_as_hex(const void *buffer, size_t length);

//! @brief Skip response, use in callback in twr_atci_command_t

bool twr_atci_skip_response(void);

//! @brief Helper for clac action

bool twr_atci_clac_action(void);

//! @brief Helper for help action

bool twr_atci_help_action(void);

//! @brief Parse string to uint and move parsing cursor forward
//! @param[in] param ATCI instance
//! @param[in] value pointer to number
//! @return true On success
//! @return false On failure

bool twr_atci_get_uint(twr_atci_param_t *param, uint32_t *value);

//! @brief Copy string and move parsing cursor forward
//! @param[in] param ATCI instance
//! @param[in] value pointer to str destination
//! @param[in] length maximum str length
//! @return true On success
//! @return false On failure

bool twr_atci_get_string(twr_atci_param_t *param, char *str, size_t length);

//! @brief Decode HEX string to buffer and move parsing cursor forward
//! @param[in] param ATCI instance
//! @param[out] destination Pointer to destination buffer
//! @param[in,out] length Number of bytes to be read, Number of bytes read
//! @return true On success
//! @return false On failure

bool twr_atci_get_buffer_from_hex_string(twr_atci_param_t *param, void *buffer, size_t *length);

//! @brief Check if the character at cursor is comma
//! @param[in] param ATCI instance
//! @return true On success
//! @return false On failure

bool twr_atci_is_comma(twr_atci_param_t *param);

//! @brief Check if the character at cursor is quotation mark (")
//! @param[in] param ATCI instance
//! @return true On success
//! @return false On failure

bool twr_atci_is_quotation_mark(twr_atci_param_t *param);

//! @brief @brief Set callback function for scan if uart is active. Used for low-power when USB is disconnected (default callback: twr_system_get_vbus_sense, scan_interval: 200)
//! @param[in] callback Callback function address
//! @param[in] scan_interval Desired scan interval in ticks

void twr_atci_set_uart_active_callback(bool(*callback)(void), twr_tick_t scan_interval);

//! @}

#endif //_TWR_ATCI_H
