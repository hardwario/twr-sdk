#ifndef _BC_LOG_H
#define _BC_LOG_H

#include <bc_common.h>

//! @addtogroup bc_log bc_log
//! @brief Logging facility (output at TXD2, parameters 115200 / 8N1)
//! @{

//! @brief Log level

typedef enum
{
    //! @brief Logging disable
    BC_LOG_LEVEL_OFF = -1,

    //! @brief Log level DEBUG
    BC_LOG_LEVEL_DEBUG = 0,

    //! @brief Log level INFO
    BC_LOG_LEVEL_INFO = 1,

    //! @brief Log level WARNING
    BC_LOG_LEVEL_WARNING = 2,

    //! @brief Log level ERROR
    BC_LOG_LEVEL_ERROR = 3

} bc_log_level_t;

//! @brief Log timestamp

typedef enum
{
    //! @brief Timestamp logging disable
    BC_LOG_TIMESTAMP_OFF = -1,

    //! @brief Timestamp logging enabled (absolute time format)
    BC_LOG_TIMESTAMP_ABS = 0,

    //! @brief Timestamp logging enabled (relative time format)
    BC_LOG_TIMESTAMP_REL = 1

} bc_log_timestamp_t;

#ifdef DEBUG

//! @brief Initialize logging library
//! @param[in] level Minimum required message level for propagation

void bc_log_init(bc_log_level_t level, bc_log_timestamp_t timestamp);

//! @brief Log DEBUG message (annotated in log as <D>)
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments

void bc_log_debug(const char *format, ...);

//! @brief Log INFO message (annotated in log as <I>)
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments

void bc_log_info(const char *format, ...);

//! @brief Log WARNING message (annotated in log as <W>)
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments

void bc_log_warning(const char *format, ...);

//! @brief Log ERROR message (annotated in log as <E>)
//! @param[in] format Format string (printf style)
//! @param[in] ... Optional format arguments

void bc_log_error(const char *format, ...);

#else

#define bc_log_debug(...)
#define bc_log_info(...)
#define bc_log_warning(...)
#define bc_log_error(...)

#endif

//! @}

#endif // _BC_LOG_H
