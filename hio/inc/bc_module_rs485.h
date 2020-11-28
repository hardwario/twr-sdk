#ifndef _HIO_MODULE_RS485_H
#define _HIO_MODULE_RS485_H

#include "hio_tick.h"
#include "hio_sc16is740.h"
#include "hio_scheduler.h"
#include "hio_fifo.h"

//! @addtogroup hio_module_rs485 hio_module_rs485
//! @brief Driver for RS-485 Module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_MODULE_RS485_EVENT_ERROR = 0,

    //! @brief Update event
    HIO_MODULE_RS485_EVENT_VOLTAGE = 1,

    //! @brief Async write done
    HIO_MODULE_RS485_EVENT_ASYNC_WRITE_DONE = 2,

    //! @brief Reading done event
    HIO_MODULE_RS485_EVENT_ASYNC_READ_DATA = 2,

    //! @brief Timeout event
    HIO_MODULE_RS485_EVENT_ASYNC_READ_TIMEOUT = 3

} hio_module_rs485_event_t;

//! @brief Baudrates

typedef enum
{
    HIO_MODULE_RS485_BAUDRATE_9600 = HIO_SC16IS740_BAUDRATE_9600,
    HIO_MODULE_RS485_BAUDRATE_19200 = HIO_SC16IS740_BAUDRATE_19200,
    HIO_MODULE_RS485_BAUDRATE_38400 = HIO_SC16IS740_BAUDRATE_38400,
    HIO_MODULE_RS485_BAUDRATE_57600 = HIO_SC16IS740_BAUDRATE_57600,
    HIO_MODULE_RS485_BAUDRATE_115200 = HIO_SC16IS740_BAUDRATE_115200

} hio_module_rs485_baudrate_t;

//! @brief Initialize RS-485 Module
//! @return true On success
//! @return false When module is not detected

bool hio_module_rs485_init(void);

//! @brief Deinitialize RS-485 Module
//! @return true On success
//! @return false When module is not detected

bool hio_module_rs485_deinit(void);

//! @brief Start single voltage measurement
//! @return true On success
//! @return false When other measurement is in progress

bool hio_module_rs485_measure(void);

//! @brief Get measured voltage
//! @param[out] volt Measured voltage in volts

bool hio_module_rs485_get_voltage(float *volt);

//! @brief Set callback function
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_module_rs485_set_event_handler(void (*event_handler)(hio_module_rs485_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] interval Measurement interval

void hio_module_rs485_set_update_interval(hio_tick_t interval);

//! @brief Write data to RS-485 bus
//! @param[in] buffer Data to send
//! @param[in] interval Number of bytes to send

size_t hio_module_rs485_write(uint8_t *buffer, size_t length);

//! @brief Get number of received bytes
//! @param[out] available Number of received bytes

bool hio_module_rs485_available(size_t *available);

//! @brief Read the received data
//! @param[out] buffer Pointer to the buffer where the data will be copied
//! @param[in] length Size of the buffer
//! @param[in] timeout Receive timeout. Write 0 for no timeout

size_t hio_module_rs485_read(uint8_t *buffer, size_t length, hio_tick_t timeout);

//! @brief Set baudrate
//! @param[in] self Instance
//! @param[in] baudrate
//! @return true On success
//! @return false On failure

bool hio_module_rs485_set_baudrate(hio_module_rs485_baudrate_t baudrate);

//! @brief Set FIFO
//! @param[in] write_fifo Pointer to the created and initialized write FIFO
//! @param[in] read_fifo Pointer to the created and initialized read FIFO

void hio_module_rs485_set_async_fifo(hio_fifo_t *write_fifo, hio_fifo_t *read_fifo);

//! @brief Add data to be transmited in async mode
//! @param[in] buffer Pointer to buffer
//! @param[in] length Length of data to be added
//! @return Number of bytes added

size_t hio_module_rs485_async_write(uint8_t *buffer, size_t length);

//! @brief Start async reading
//! @param[in] timeout Maximum timeout in ms
//! @return true On success
//! @return false On failure

bool hio_module_rs485_async_read_start(hio_tick_t timeout);

//! @brief Stop async reading
//! @return true On success
//! @return false On failure

bool hio_module_rs485_async_read_stop(void);

//! @brief Get data that has been received in async mode
//! @param[in] buffer Pointer to buffer
//! @param[in] length Maximum length of received data
//! @return Number of received bytes

size_t hio_module_rs485_async_read(void *buffer, size_t length);

//! @}

#endif
