#ifndef _BC_LP8_H
#define _BC_LP8_H

#include <bc_scheduler.h>
#include <bc_tick.h>

//! @addtogroup bc_lp8 bc_lp8
//! @brief Driver for LP8 CO2 sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_LP8_EVENT_ERROR = 0,

    //! @brief Update event
    BC_LP8_EVENT_UPDATE = 1

} bc_lp8_event_t;

//! @brief Calibration

typedef enum
{
    //! @brief Background calibration using unfiltered data
    BC_LP8_CALIBRATION_BACKGROUND_UNFILTERED = 0x50,

    //! @brief Background calibration using filtered data
    BC_LP8_CALIBRATION_BACKGROUND_FILTERED = 0x51,

    //! @brief Background calibration using unfiltered data + reset filters
    BC_LP8_CALIBRATION_BACKGROUND_UNFILTERED_RF = 0x52,

    //! @brief Background calibration using filtered data + reset filters
    BC_LP8_CALIBRATION_BACKGROUND_FILTERED_RF = 0x53,

    //! @brief ABC (based on filtered data)
    BC_LP8_CALIBRATION_ABC = 0x70,

    //! @brief ABC (based on filtered data) + reset filters
    BC_LP8_CALIBRATION_ABC_RF = 0x72

} bc_lp8_calibration_t;

//! @brief LP8 instance

typedef struct bc_lp8_t bc_lp8_t;

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

} bc_lp8_driver_t;

//! @cond

typedef enum
{
    BC_LP8_STATE_ERROR = -1,
    BC_LP8_STATE_INITIALIZE = 0,
    BC_LP8_STATE_READY = 1,
    BC_LP8_STATE_PRECHARGE = 2,
    BC_LP8_STATE_CHARGE = 3,
    BC_LP8_STATE_BOOT = 4,
    BC_LP8_STATE_BOOT_READ = 5,
    BC_LP8_STATE_MEASURE = 6,
    BC_LP8_STATE_MEASURE_READ = 7

} bc_lp8_state_t;

typedef enum
{
    BC_LP8_ERROR_INITIALIZE = 0,
    BC_LP8_ERROR_PRECHARGE = 1,

    BC_LP8_ERROR_CHARGE_CHARGE_ENABLE = 2,
    BC_LP8_ERROR_CHARGE_DEVICE_ENABLE = 3,

    BC_LP8_ERROR_BOOT_SIGNAL_READY = 4,
    BC_LP8_ERROR_BOOT_TIMEOUT = 5,
    BC_LP8_ERROR_BOOT_UART_ENABLE = 6,
    BC_LP8_ERROR_BOOT_UART_WRITE = 7,

    BC_LP8_ERROR_BOOT_READ_UART_ENABLE = 8,
    BC_LP8_ERROR_BOOT_READ_DEVICE_ADDRESS = 9,
    BC_LP8_ERROR_BOOT_READ_COMMAND = 10,
    BC_LP8_ERROR_BOOT_READ_CRC = 11,
    BC_LP8_ERROR_BOOT_READ_TIMEOUT = 12,

    BC_LP8_ERROR_MEASURE_SIGNAL_RDY = 13,
    BC_LP8_ERROR_MEASURE_SIGNAL_RDY_TIMEOUT = 14,
    BC_LP8_ERROR_MEASURE_UART_ENABLE = 15,
    BC_LP8_ERROR_MEASURE_UART_WRITE = 16,

    BC_LP8_ERROR_MEASURE_READ_UART_ENABLE = 17,
    BC_LP8_ERROR_MEASURE_READ_DEVICE_ENABLE = 18,
    BC_LP8_ERROR_MEASURE_READ_DEVICE_ADDRESS = 19,
    BC_LP8_ERROR_MEASURE_READ_COMMAND = 20,
    BC_LP8_ERROR_MEASURE_READ_CRC = 21,
    BC_LP8_ERROR_MEASURE_READ_CALIBRATION_RUN = 22,
    BC_LP8_ERROR_MEASURE_READ_STATUS1 = 23,
    BC_LP8_ERROR_MEASURE_READ_TIMEOUT = 24

} bc_lp8_error_t;

struct bc_lp8_t
{
    const bc_lp8_driver_t *_driver;
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(bc_lp8_event_t, void *);
    void *_event_param;
    bc_tick_t _update_interval;
    bc_lp8_state_t _state;
    bool _first_measurement_done;
    bc_tick_t _tick_timeout;
    bc_tick_t _tick_calibration;
    bc_lp8_calibration_t _calibration;
    bool _calibration_run;
    uint8_t _rx_buffer[49];
    size_t _rx_buffer_length;
    uint8_t _tx_buffer[33];
    uint8_t _sensor_state[23];
    bool _valid;
    int16_t _concentration;
    uint16_t _pressure;
    bc_lp8_error_t _error;

};

//! @endcond

//! @brief Initialize LP8

void bc_lp8_init(bc_lp8_t *self, const bc_lp8_driver_t *driver);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_lp8_set_event_handler(bc_lp8_t *self, void (*event_handler)(bc_lp8_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_lp8_set_update_interval(bc_lp8_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return False When other measurement is in progress

bool bc_lp8_measure(bc_lp8_t *self);

//! @brief Get CO2 concentration in ppm (parts per million)
//! @param[in] self Instance
//! @param[out] ppm Pointer to variable where result will be stored
//! @return true On success
//! @return false On failure

bool bc_lp8_get_concentration_ppm(bc_lp8_t *self, float *ppm);

//! @brief Get last error code
//! @param[in] self Instance
//! @param[out] pointer to the variable where error code will be stored
//! @return true On success
//! @return false On failure

bool bc_lp8_get_error(bc_lp8_t *self, bc_lp8_error_t *error);

//! @brief Request sensor calibration
//! @param[in] self Instance
//! @param[in] calibration Calibration type

void bc_lp8_calibration(bc_lp8_t *self, bc_lp8_calibration_t calibration);

//! @}

#endif // _BC_LP8_H
