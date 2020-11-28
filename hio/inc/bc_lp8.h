#ifndef _HIO_LP8_H
#define _HIO_LP8_H

#include <hio_scheduler.h>
#include <hio_tick.h>

//! @addtogroup hio_lp8 hio_lp8
//! @brief Driver for LP8 CO2 sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_LP8_EVENT_ERROR = 0,

    //! @brief Update event
    HIO_LP8_EVENT_UPDATE = 1

} hio_lp8_event_t;

//! @brief Calibration

typedef enum
{
    //! @brief Background calibration using unfiltered data
    HIO_LP8_CALIBRATION_BACKGROUND_UNFILTERED = 0x50,

    //! @brief Background calibration using filtered data
    HIO_LP8_CALIBRATION_BACKGROUND_FILTERED = 0x51,

    //! @brief Background calibration using unfiltered data + reset filters
    HIO_LP8_CALIBRATION_BACKGROUND_UNFILTERED_RF = 0x52,

    //! @brief Background calibration using filtered data + reset filters
    HIO_LP8_CALIBRATION_BACKGROUND_FILTERED_RF = 0x53,

    //! @brief ABC (based on filtered data)
    HIO_LP8_CALIBRATION_ABC = 0x70,

    //! @brief ABC (based on filtered data) + reset filters
    HIO_LP8_CALIBRATION_AHIO_RF = 0x72

} hio_lp8_calibration_t;

//! @brief LP8 instance

typedef struct hio_lp8_t hio_lp8_t;

//! @brief LP8 driver

typedef struct
{
    //! @brief Callback for initialization
    bool (*init)(void);

    //! @brief Callback for charging control
    bool (*charge_enable)(bool state);

    //! @brief Callback for device power control
    bool (*device_enable)(bool state);

    //! @brief Callback for reading RDY signal
    bool (*read_signal_rdy)(int *value);

    //! @brief Callback for UART control
    bool (*uart_enable)(bool state);

    //! @brief Callback for UART write
    size_t (*uart_write)(uint8_t *buffer, size_t length);

    //! @brief Callback for UART read
    size_t (*uart_read)(uint8_t *buffer, size_t length);

} hio_lp8_driver_t;

//! @cond

typedef enum
{
    HIO_LP8_STATE_ERROR = -1,
    HIO_LP8_STATE_INITIALIZE = 0,
    HIO_LP8_STATE_READY = 1,
    HIO_LP8_STATE_PRECHARGE = 2,
    HIO_LP8_STATE_CHARGE = 3,
    HIO_LP8_STATE_BOOT = 4,
    HIO_LP8_STATE_BOOT_READ = 5,
    HIO_LP8_STATE_MEASURE = 6,
    HIO_LP8_STATE_MEASURE_READ = 7

} hio_lp8_state_t;

typedef enum
{
    HIO_LP8_ERROR_INITIALIZE = 0,
    HIO_LP8_ERROR_PRECHARGE = 1,

    HIO_LP8_ERROR_CHARGE_CHARGE_ENABLE = 2,
    HIO_LP8_ERROR_CHARGE_DEVICE_ENABLE = 3,

    HIO_LP8_ERROR_BOOT_SIGNAL_READY = 4,
    HIO_LP8_ERROR_BOOT_TIMEOUT = 5,
    HIO_LP8_ERROR_BOOT_UART_ENABLE = 6,
    HIO_LP8_ERROR_BOOT_UART_WRITE = 7,

    HIO_LP8_ERROR_BOOT_READ_UART_ENABLE = 8,
    HIO_LP8_ERROR_BOOT_READ_DEVICE_ADDRESS = 9,
    HIO_LP8_ERROR_BOOT_READ_COMMAND = 10,
    HIO_LP8_ERROR_BOOT_READ_CRC = 11,
    HIO_LP8_ERROR_BOOT_READ_TIMEOUT = 12,

    HIO_LP8_ERROR_MEASURE_SIGNAL_RDY = 13,
    HIO_LP8_ERROR_MEASURE_SIGNAL_RDY_TIMEOUT = 14,
    HIO_LP8_ERROR_MEASURE_UART_ENABLE = 15,
    HIO_LP8_ERROR_MEASURE_UART_WRITE = 16,

    HIO_LP8_ERROR_MEASURE_READ_UART_ENABLE = 17,
    HIO_LP8_ERROR_MEASURE_READ_DEVICE_ENABLE = 18,
    HIO_LP8_ERROR_MEASURE_READ_DEVICE_ADDRESS = 19,
    HIO_LP8_ERROR_MEASURE_READ_COMMAND = 20,
    HIO_LP8_ERROR_MEASURE_READ_CRC = 21,
    HIO_LP8_ERROR_MEASURE_READ_CALIBRATION_RUN = 22,
    HIO_LP8_ERROR_MEASURE_READ_STATUS1 = 23,
    HIO_LP8_ERROR_MEASURE_READ_TIMEOUT = 24

} hio_lp8_error_t;

struct hio_lp8_t
{
    const hio_lp8_driver_t *_driver;
    hio_scheduler_task_id_t _task_id_interval;
    hio_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(hio_lp8_event_t, void *);
    void *_event_param;
    hio_tick_t _update_interval;
    hio_lp8_state_t _state;
    bool _first_measurement_done;
    hio_tick_t _tick_timeout;
    hio_tick_t _tick_calibration;
    hio_lp8_calibration_t _calibration;
    bool _calibration_run;
    uint8_t _rx_buffer[49];
    size_t _rx_buffer_length;
    uint8_t _tx_buffer[33];
    uint8_t _sensor_state[23];
    bool _valid;
    int16_t _concentration;
    uint16_t _pressure;
    hio_lp8_error_t _error;

};

//! @endcond

//! @brief Initialize LP8

void hio_lp8_init(hio_lp8_t *self, const hio_lp8_driver_t *driver);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_lp8_set_event_handler(hio_lp8_t *self, void (*event_handler)(hio_lp8_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void hio_lp8_set_update_interval(hio_lp8_t *self, hio_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return False When other measurement is in progress

bool hio_lp8_measure(hio_lp8_t *self);

//! @brief Get CO2 concentration in ppm (parts per million)
//! @param[in] self Instance
//! @param[out] ppm Pointer to variable where result will be stored
//! @return true On success
//! @return false On failure

bool hio_lp8_get_concentration_ppm(hio_lp8_t *self, float *ppm);

//! @brief Get last error code
//! @param[in] self Instance
//! @param[out] pointer to the variable where error code will be stored
//! @return true On success
//! @return false On failure

bool hio_lp8_get_error(hio_lp8_t *self, hio_lp8_error_t *error);

//! @brief Request sensor calibration
//! @param[in] self Instance
//! @param[in] calibration Calibration type

void hio_lp8_calibration(hio_lp8_t *self, hio_lp8_calibration_t calibration);

//! @}

#endif // _HIO_LP8_H
