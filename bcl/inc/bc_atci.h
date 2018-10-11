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

//! @brief Helper for clac action

bool bc_atci_clac_action(void);

//! @brief Helper for help action

bool bc_atci_help_action(void);

//! @}

#endif //_BC_ATCI_H
