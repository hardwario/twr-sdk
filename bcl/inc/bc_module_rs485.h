#ifndef _BC_MODULE_RS485_H
#define _BC_MODULE_RS485_H

#include "bc_tick.h"
#include "bc_sc16is740.h"
#include "bc_scheduler.h"
#include "bc_fifo.h"

//! @addtogroup bc_module_rs485 bc_module_rs485
//! @brief Driver for RS-485 Module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_MODULE_RS485_EVENT_ERROR = 0,

    //! @brief Update event
    BC_MODULE_RS485_EVENT_VOLTAGE = 1,

    //! @brief Async write done
    BC_MODULE_RS485_EVENT_ASYNC_WRITE_DONE = 2,

    //! @brief Reading done event
    BC_MODULE_RS485_EVENT_ASYNC_READ_DATA = 2,

    //! @brief Timeout event
    BC_MODULE_RS485_EVENT_ASYNC_READ_TIMEOUT = 3

} bc_module_rs485_event_t;

//! @brief Baudrates

typedef enum
{
    BC_MODULE_RS485_BAUDRATE_9600 = BC_SC16IS740_BAUDRATE_9600,
    BC_MODULE_RS485_BAUDRATE_19200 = BC_SC16IS740_BAUDRATE_19200,
    BC_MODULE_RS485_BAUDRATE_38400 = BC_SC16IS740_BAUDRATE_38400,
    BC_MODULE_RS485_BAUDRATE_57600 = BC_SC16IS740_BAUDRATE_57600,
    BC_MODULE_RS485_BAUDRATE_115200 = BC_SC16IS740_BAUDRATE_115200

} bc_module_rs485_baudrate_t;

//! @brief Initialize RS-485 Module
//! @return true On success
//! @return false When module is not detected

bool bc_module_rs485_init(void);

//! @brief Start single voltage measurement
//! @return true On success
//! @return false When other measurement is in progress

bool bc_module_rs485_measure(void);

//! @brief Get measured voltage
//! @param[out] volt Measured voltage in volts

bool bc_module_rs485_get_voltage(float *volt);

//! @brief Set callback function
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_module_rs485_set_event_handler(void (*event_handler)(bc_module_rs485_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] interval Measurement interval

void bc_module_rs485_set_update_interval(bc_tick_t interval);

//! @brief Write data to RS-485 bus
//! @param[in] buffer Data to send
//! @param[in] interval Number of bytes to send

size_t bc_module_rs485_write(uint8_t *buffer, size_t length);

//! @brief Get number of received bytes
//! @param[out] available Number of received bytes

bool bc_module_rs485_available(size_t *available);

//! @brief Read the received data
//! @param[out] buffer Pointer to the buffer where the data will be copied
//! @param[in] length Size of the buffer
//! @param[in] timeout Receive timeout. Write 0 for no timeout

size_t bc_module_rs485_read(uint8_t *buffer, size_t length, bc_tick_t timeout);

//! @brief Set baudrate
//! @param[in] self Instance
//! @param[in] baudrate
//! @return true On success
//! @return false On failure

bool bc_module_rs485_set_baudrate(bc_module_rs485_baudrate_t baudrate);

//! @brief Set FIFO
//! @param[in] write_fifo Pointer to the created and initialized write FIFO
//! @param[in] read_fifo Pointer to the created and initialized read FIFO

void bc_module_rs485_set_async_fifo(bc_fifo_t *write_fifo, bc_fifo_t *read_fifo);

//! @brief Add data to be transmited in async mode
//! @param[in] buffer Pointer to buffer
//! @param[in] length Length of data to be added
//! @return Number of bytes added

size_t bc_module_rs485_async_write(uint8_t *buffer, size_t length);

//! @brief Start async reading
//! @param[in] timeout Maximum timeout in ms
//! @return true On success
//! @return false On failure

bool bc_module_rs485_async_read_start(bc_tick_t timeout);

//! @brief Stop async reading
//! @return true On success
//! @return false On failure

bool bc_module_rs485_async_read_stop(void);

//! @brief Get data that has been received in async mode
//! @param[in] buffer Pointer to buffer
//! @param[in] length Maximum length of received data
//! @return Number of received bytes

size_t bc_module_rs485_async_read(void *buffer, size_t length);

//! @}

#endif
