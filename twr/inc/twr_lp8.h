#ifndef _TWR_LP8_H
#define _TWR_LP8_H

#include <twr_scheduler.h>
#include <twr_tick.h>

//! @addtogroup twr_lp8 twr_lp8
//! @brief Driver for LP8 CO2 sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_LP8_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_LP8_EVENT_UPDATE = 1

} twr_lp8_event_t;

//! @brief Calibration

typedef enum
{
    //! @brief Background calibration using unfiltered data
    TWR_LP8_CALIBRATION_BACKGROUND_UNFILTERED = 0x50,

    //! @brief Background calibration using filtered data
    TWR_LP8_CALIBRATION_BACKGROUND_FILTERED = 0x51,

    //! @brief Background calibration using unfiltered data + reset filters
    TWR_LP8_CALIBRATION_BACKGROUND_UNFILTERED_RF = 0x52,

    //! @brief Background calibration using filtered data + reset filters
    TWR_LP8_CALIBRATION_BACKGROUND_FILTERED_RF = 0x53,

    //! @brief ABC (based on filtered data)
    TWR_LP8_CALIBRATION_ABC = 0x70,

    //! @brief ABC (based on filtered data) + reset filters
    TWR_LP8_CALIBRATION_ATWR_RF = 0x72

} twr_lp8_calibration_t;

//! @brief LP8 instance

typedef struct twr_lp8_t twr_lp8_t;

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

} twr_lp8_driver_t;

//! @cond

typedef enum
{
    TWR_LP8_STATE_ERROR = -1,
    TWR_LP8_STATE_INITIALIZE = 0,
    TWR_LP8_STATE_READY = 1,
    TWR_LP8_STATE_PRECHARGE = 2,
    TWR_LP8_STATE_CHARGE = 3,
    TWR_LP8_STATE_BOOT = 4,
    TWR_LP8_STATE_BOOT_READ = 5,
    TWR_LP8_STATE_MEASURE = 6,
    TWR_LP8_STATE_MEASURE_READ = 7

} twr_lp8_state_t;

typedef enum
{
    TWR_LP8_ERROR_INITIALIZE = 0,
    TWR_LP8_ERROR_PRECHARGE = 1,

    TWR_LP8_ERROR_CHARGE_CHARGE_ENABLE = 2,
    TWR_LP8_ERROR_CHARGE_DEVICE_ENABLE = 3,

    TWR_LP8_ERROR_BOOT_SIGNAL_READY = 4,
    TWR_LP8_ERROR_BOOT_TIMEOUT = 5,
    TWR_LP8_ERROR_BOOT_UART_ENABLE = 6,
    TWR_LP8_ERROR_BOOT_UART_WRITE = 7,

    TWR_LP8_ERROR_BOOT_READ_UART_ENABLE = 8,
    TWR_LP8_ERROR_BOOT_READ_DEVICE_ADDRESS = 9,
    TWR_LP8_ERROR_BOOT_READ_COMMAND = 10,
    TWR_LP8_ERROR_BOOT_READ_CRC = 11,
    TWR_LP8_ERROR_BOOT_READ_TIMEOUT = 12,

    TWR_LP8_ERROR_MEASURE_SIGNAL_RDY = 13,
    TWR_LP8_ERROR_MEASURE_SIGNAL_RDY_TIMEOUT = 14,
    TWR_LP8_ERROR_MEASURE_UART_ENABLE = 15,
    TWR_LP8_ERROR_MEASURE_UART_WRITE = 16,

    TWR_LP8_ERROR_MEASURE_READ_UART_ENABLE = 17,
    TWR_LP8_ERROR_MEASURE_READ_DEVICE_ENABLE = 18,
    TWR_LP8_ERROR_MEASURE_READ_DEVICE_ADDRESS = 19,
    TWR_LP8_ERROR_MEASURE_READ_COMMAND = 20,
    TWR_LP8_ERROR_MEASURE_READ_CRC = 21,
    TWR_LP8_ERROR_MEASURE_READ_CALIBRATION_RUN = 22,
    TWR_LP8_ERROR_MEASURE_READ_STATUS1 = 23,
    TWR_LP8_ERROR_MEASURE_READ_TIMEOUT = 24

} twr_lp8_error_t;

struct twr_lp8_t
{
    const twr_lp8_driver_t *_driver;
    twr_scheduler_task_id_t _task_id_interval;
    twr_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(twr_lp8_event_t, void *);
    void *_event_param;
    twr_tick_t _update_interval;
    twr_lp8_state_t _state;
    bool _first_measurement_done;
    twr_tick_t _tick_timeout;
    twr_tick_t _tick_calibration;
    twr_lp8_calibration_t _calibration;
    bool _calibration_run;
    uint8_t _rx_buffer[49];
    size_t _rx_buffer_length;
    uint8_t _tx_buffer[33];
    uint8_t _sensor_state[23];
    bool _valid;
    int16_t _concentration;
    uint16_t _pressure;
    twr_lp8_error_t _error;

};

//! @endcond

//! @brief Initialize LP8

void twr_lp8_init(twr_lp8_t *self, const twr_lp8_driver_t *driver);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_lp8_set_event_handler(twr_lp8_t *self, void (*event_handler)(twr_lp8_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_lp8_set_update_interval(twr_lp8_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return False When other measurement is in progress

bool twr_lp8_measure(twr_lp8_t *self);

//! @brief Get CO2 concentration in ppm (parts per million)
//! @param[in] self Instance
//! @param[out] ppm Pointer to variable where result will be stored
//! @return true On success
//! @return false On failure

bool twr_lp8_get_concentration_ppm(twr_lp8_t *self, float *ppm);

//! @brief Get last error code
//! @param[in] self Instance
//! @param[out] pointer to the variable where error code will be stored
//! @return true On success
//! @return false On failure

bool twr_lp8_get_error(twr_lp8_t *self, twr_lp8_error_t *error);

//! @brief Request sensor calibration
//! @param[in] self Instance
//! @param[in] calibration Calibration type

void twr_lp8_calibration(twr_lp8_t *self, twr_lp8_calibration_t calibration);

//! @}

#endif // _TWR_LP8_H
