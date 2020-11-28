#ifndef _HIO_LOG_H
#define _HIO_LOG_H

#include <hio_common.h>
#include <hio_uart.h>

//! @addtogroup hio_log hio_log
//! @brief Logging facility (output on TXD2, format 115200 / 8N1)
//! @{

#ifndef HIO_LOG_UART
#define HIO_LOG_UART       HIO_UART_UART2
#endif

#define HIO_LOG_DUMP_WIDTH 8

//! @brief Log level

typedef enum
{
    //! @brief Logging DUMP
    HIO_LOG_LEVEL_DUMP = 0,

    //! @brief Log level DEBUG
    HIO_LOG_LEVEL_DEBUG = 1,

    //! @brief Log level INFO
    HIO_LOG_LEVEL_INFO = 2,

    //! @brief Log level WARNING
    HIO_LOG_LEVEL_WARNING = 3,

    //! @brief Log level ERROR
    HIO_LOG_LEVEL_ERROR = 4,

    //! @brief Logging disabled
    HIO_LOG_LEVEL_OFF = 5

} hio_log_level_t;

//! @brief Log timestamp

typedef enum
{
    //! @brief Timestamp logging disabled
    HIO_LOG_TIMESTAMP_OFF = -1,

    //! @brief Timestamp logging enabled (absolute time format)
    HIO_LOG_TIMESTAMP_ABS = 0,

    //! @brief Timestamp logging enabled (relative time format)
    HIO_LOG_TIMESTAMP_REL = 1

} hio_log_timestamp_t;

#ifndef RELEASE

//! @brief Initialize logging facility
//! @param[in] level Minimum required message level for propagation
//! @param[in] timestamp Timestamp logging setting

void hio_log_init(hio_log_level_t level, hio_log_timestamp_t timestamp);

//! @brief Log DUMP message (annotated in log as <X>)
//! @param[in] buffer Pointer to source buffer
//! @param[in] length Number of bytes to be printed
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments

void hio_log_dump(const void *buffer, size_t length, const char *format, ...) __attribute__ ((format (printf, 3, 4)));

//! @brief Log DEBUG message (annotated in log as <D>)
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments

void hio_log_debug(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

//! @brief Log INFO message (annotated in log as <I>)
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments

void hio_log_info(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

//! @brief Log WARNING message (annotated in log as <W>)
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments

void hio_log_warning(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

//! @brief Log ERROR message (annotated in log as <E>)
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments

void hio_log_error(const char *format, ...) __attribute__ ((format (printf, 1, 2)));

#else

#define hio_log_init(...)
#define hio_log_dump(...)
#define hio_log_debug(...)
#define hio_log_info(...)
#define hio_log_warning(...)
#define hio_log_error(...)

#endif

//! @}

#endif // _HIO_LOG_H
