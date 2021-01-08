#ifndef _TWR_LOG_H
#define _TWR_LOG_H

#include <twr_common.h>
#include <twr_uart.h>

//! @addtogroup twr_log twr_log
//! @brief Logging facility (output on TXD2, format 115200 / 8N1)
//! @{

#ifndef TWR_LOG_UART
#define TWR_LOG_UART       TWR_UART_UART2
#endif

#ifndef TWR_LOG_BUFFER_SIZE
#define TWR_LOG_BUFFER_SIZE 256
#endif

#define TWR_LOG_DUMP_WIDTH 8

//! @brief Log level

typedef enum
{
    //! @brief Logging DUMP
    TWR_LOG_LEVEL_DUMP = 0,

    //! @brief Log level DEBUG
    TWR_LOG_LEVEL_DEBUG = 1,

    //! @brief Log level INFO
    TWR_LOG_LEVEL_INFO = 2,

    //! @brief Log level WARNING
    TWR_LOG_LEVEL_WARNING = 3,

    //! @brief Log level ERROR
    TWR_LOG_LEVEL_ERROR = 4,

    //! @brief Logging disabled
    TWR_LOG_LEVEL_OFF = 5

} twr_log_level_t;

//! @brief Log timestamp

typedef enum
{
    //! @brief Timestamp logging disabled
    TWR_LOG_TIMESTAMP_OFF = -1,

    //! @brief Timestamp logging enabled (absolute time format)
    TWR_LOG_TIMESTAMP_ABS = 0,

    //! @brief Timestamp logging enabled (relative time format)
    TWR_LOG_TIMESTAMP_REL = 1

} twr_log_timestamp_t;

#ifndef RELEASE

//! @brief Initialize logging facility
//! @param[in] level Minimum required message level for propagation
//! @param[in] timestamp Timestamp logging setting

void twr_log_init(twr_log_level_t level, twr_log_timestamp_t timestamp);

//! @brief Log DUMP message (annotated in log as <X>)
//! @param[in] buffer Pointer to source buffer
//! @param[in] length Number of bytes to be printed
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments

void twr_log_dump(const void *buffer, size_t length, const char *format, ...) __attribute__ ((format (printf, 3, 4)));

//! @brief Log DEBUG message (annotated in log as <D>)
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments

void twr_log_debug(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

//! @brief Log INFO message (annotated in log as <I>)
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments

void twr_log_info(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

//! @brief Log WARNING message (annotated in log as <W>)
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments

void twr_log_warning(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

//! @brief Log ERROR message (annotated in log as <E>)
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments

void twr_log_error(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

#else

#define twr_log_init(...)
#define twr_log_dump(...)
#define twr_log_debug(...)
#define twr_log_info(...)
#define twr_log_warning(...)
#define twr_log_error(...)

#endif

//! @}

#endif // _TWR_LOG_H
