#ifndef _HIO_ATCI_H
#define _HIO_ATCI_H

#include <hio_uart.h>

//! @addtogroup hio_atci hio_atci
//! @brief AT command interface
//! @{

#ifndef HIO_ATCI_UART
#define HIO_ATCI_UART HIO_UART_UART2
#endif

#define HIO_ATCI_COMMANDS_LENGTH(COMMANDS) (sizeof(COMMANDS) / sizeof(COMMANDS[0]))

#define HIO_ATCI_COMMAND_CLAC {"+CLAC", hio_atci_clac_action, NULL, NULL, NULL, ""}
#define HIO_ATCI_COMMAND_HELP {"$HELP", hio_atci_help_action, NULL, NULL, NULL, "This help"}

typedef struct
{
    char *txt;
    size_t length;
    size_t offset;

} hio_atci_param_t;

//! @brief AT command struct

typedef struct
{
  const char *command;
  bool (*action)(void);
  bool (*set)(hio_atci_param_t *param);
  bool (*read)(void);
  bool (*help)(void);
  const char *hint;

} hio_atci_command_t;

//! @brief Initialize
//! @param[in] commands
//! @param[in] length Number of commands

void hio_atci_init(const hio_atci_command_t *commands, int length);

//! @brief Write OK

void hio_atci_write_ok(void);

//! @brief Write ERROR

void hio_atci_write_error(void);

//! @brief Prinf message and add CR LF
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments

void hio_atci_printf(const char *format, ...);

//! @brief Skip response, use in callback in hio_atci_command_t

bool hio_atci_skip_response(void);

//! @brief Helper for clac action

bool hio_atci_clac_action(void);

//! @brief Helper for help action

bool hio_atci_help_action(void);

//! @brief Parse string to uint and move parsing cursor forward
//! @param[in] param ATCI instance
//! @param[in] value pointer to number
//! @return true On success
//! @return false On failure

bool hio_atci_get_uint(hio_atci_param_t *param, uint32_t *value);

//! @brief Copy string and move parsing cursor forward
//! @param[in] param ATCI instance
//! @param[in] value pointer to str destination
//! @param[in] length maximum str length
//! @return true On success
//! @return false On failure

bool hio_atci_get_string(hio_atci_param_t *param, char *str, size_t length);

//! @brief Decode HEX string to buffer and move parsing cursor forward
//! @param[in] param ATCI instance
//! @param[out] destination Pointer to destination buffer
//! @param[in,out] length Number of bytes to be read, Number of bytes read
//! @return true On success
//! @return false On failure

bool hio_atci_get_buffer_from_hex_string(hio_atci_param_t *param, void *buffer, size_t *length);

//! @brief Check if the character at cursor is comma
//! @param[in] param ATCI instance
//! @return true On success
//! @return false On failure

bool hio_atci_is_comma(hio_atci_param_t *param);

//! @brief Check if the character at cursor is quotation mark (")
//! @param[in] param ATCI instance
//! @return true On success
//! @return false On failure

bool hio_atci_is_quotation_mark(hio_atci_param_t *param);

//! @brief @brief Set callback function for scan if uart is active. Used for low-power when USB is disconnected (default callback: hio_system_get_vbus_sense, scan_interval: 200)
//! @param[in] callback Callback function address
//! @param[in] scan_interval Desired scan interval in ticks

void hio_atci_set_uart_active_callback(bool(*callback)(void), hio_tick_t scan_interval);

//! @}

#endif //_HIO_ATCI_H
