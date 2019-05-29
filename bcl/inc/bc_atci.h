#ifndef _BC_ATCI_H
#define _BC_ATCI_H

#include <bc_uart.h>

//! @addtogroup bc_atci bc_atci
//! @brief AT command interface
//! @{

#ifndef BC_ATCI_UART
#define BC_ATCI_UART BC_UART_UART2
#endif

#define BC_ATCI_COMMANDS_LENGTH(COMMANDS) (sizeof(COMMANDS) / sizeof(COMMANDS[0]))

#define BC_ATCI_COMMAND_CLAC {"+CLAC", bc_atci_clac_action, NULL, NULL, NULL, ""}
#define BC_ATCI_COMMAND_HELP {"$HELP", bc_atci_help_action, NULL, NULL, NULL, "This help"}

typedef struct
{
    char *txt;
    size_t length;
    size_t offset;

} bc_atci_param_t;

//! @brief AT command struct

typedef struct
{
  const char *command;
  bool (*action)(void);
  bool (*set)(bc_atci_param_t *param);
  bool (*read)(void);
  bool (*help)(void);
  const char *hint;

} bc_atci_command_t;

//! @brief Initialize
//! @param[in] commands
//! @param[in] length Number of commands

void bc_atci_init(const bc_atci_command_t *commands, int length);

//! @brief Write OK

void bc_atci_write_ok(void);

//! @brief Write ERROR

void bc_atci_write_error(void);

//! @brief Prinf message and add CR LF
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments

void bc_atci_printf(const char *format, ...);

//! @brief Skip response, use in callback in bc_atci_command_t

bool bc_atci_skip_response(void);

//! @brief Helper for clac action

bool bc_atci_clac_action(void);

//! @brief Helper for help action

bool bc_atci_help_action(void);

//! @brief Parse string to uint and move parsing cursor forward
//! @param[in] param ATCI instance
//! @param[in] value pointer to number
//! @return true On success
//! @return false On failure

bool bc_atci_get_uint(bc_atci_param_t *param, uint32_t *value);

//! @brief Copy string and move parsing cursor forward
//! @param[in] param ATCI instance
//! @param[in] value pointer to str destination
//! @param[in] length maximum str length
//! @return true On success
//! @return false On failure

bool bc_atci_get_string(bc_atci_param_t *param, char *str, size_t length);

//! @brief Decode HEX string to buffer and move parsing cursor forward
//! @param[in] param ATCI instance
//! @param[out] destination Pointer to destination buffer
//! @param[in,out] length Number of bytes to be read, Number of bytes read
//! @return true On success
//! @return false On failure

bool bc_atci_get_buffer_from_hex_string(bc_atci_param_t *param, void *buffer, size_t *length);

//! @brief Check if the character at cursor is comma
//! @param[in] param ATCI instance
//! @return true On success
//! @return false On failure

bool bc_atci_is_comma(bc_atci_param_t *param);

//! @brief Check if the character at cursor is quotation mark (")
//! @param[in] param ATCI instance
//! @return true On success
//! @return false On failure

bool bc_atci_is_quotation_mark(bc_atci_param_t *param);

//! @brief @brief Set callback function for scan if uart is active. Used for low-power when USB is disconnected (default callback: bc_system_get_vbus_sense, scan_interval: 200)
//! @param[in] callback Callback function address
//! @param[in] scan_interval Desired scan interval in ticks

void bc_atci_set_uart_active_callback(bool(*callback)(void), bc_tick_t scan_interval);

//! @}

#endif //_BC_ATCI_H
