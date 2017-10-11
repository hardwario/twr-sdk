#ifndef _BC_CO2_SENSOR_H
#define _BC_CO2_SENSOR_H

#include <bc_scheduler.h>
#include <bc_tick.h>

//! @addtogroup bc_co2 bc_co2
//! @brief Driver for BigClown CO2 Sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_CO2_SENSOR_EVENT_ERROR = 0,

    //! @brief Update event
    BC_CO2_SENSOR_EVENT_UPDATE = 1

} bc_co2_sensor_event_t;

//! @brief Calibration

typedef enum
{
    //! @brief Background calibration using unfiltered data
    BC_CO2_SENSOR_CALIBRATION_BACKGROUND_UNFILTERED = 0x50,

    //! @brief Background calibration using filtered data
    BC_CO2_SENSOR_CALIBRATION_BACKGROUND_FILTERED = 0x51,

    //! @brief Background calibration using unfiltered data + reset filters
    BC_CO2_SENSOR_CALIBRATION_BACKGROUND_UNFILTERED_RF = 0x52,

    //! @brief Background calibration using filtered data + reset filters
    BC_CO2_SENSOR_CALIBRATION_BACKGROUND_FILTERED_RF = 0x53,

    //! @brief ABC (based on filtered data)
    BC_CO2_SENSOR_CALIBRATION_ABC = 0x70,

    //! @brief ABC (based on filtered data)  + reset filters
    BC_CO2_SENSOR_CALIBRATION_ABC_RF = 0x72,

} bc_co2_sensor_calibration_t;

//! @brief CO2 Sensor driver

typedef struct
{
    bool (*init)(void);
    bool (*charge)(bool state);
    bool (*enable)(bool state);
    bool (*rdy)(bool *state);
    bool (*uart_enable)(bool state);
    size_t (*uart_write)(uint8_t *buffer, size_t length);
    size_t (*uart_read)(uint8_t *buffer, size_t length);

} bc_co2_sensor_driver_t;

//! @cond

typedef enum
{
    BC_CO2_SENSOR_STATE_ERROR = -1,
    BC_CO2_SENSOR_STATE_INITIALIZE = 0,
    BC_CO2_SENSOR_STATE_READY,
    BC_CO2_SENSOR_STATE_CHARGE,
    BC_CO2_SENSOR_STATE_BOOT,
    BC_CO2_SENSOR_STATE_BOOT_READ,
    BC_CO2_SENSOR_STATE_MEASURE,
    BC_CO2_SENSOR_STATE_MEASURE_READ,

} bc_co2_sensor_state_t;

typedef struct
{
    const bc_co2_sensor_driver_t *_driver;
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(bc_co2_sensor_event_t, void *);
    void *_event_param;
    bc_tick_t _update_interval;
    bc_co2_sensor_state_t _state;
    bool _first_measurement_done;
    bc_tick_t _tick_timeout;
    bc_tick_t _tick_calibration_timeout;
    bc_co2_sensor_calibration_t _calibration;
    bool _calibration_run;
    uint8_t _rx_buffer[49];
    size_t _rx_buffer_length;
    uint8_t _tx_buffer[33];
    uint8_t _sensor_state[23];
    bool _valid;
    int16_t _concentration;
    uint16_t _pressure;

} bc_co2_sensor_t;

//! @endcond

//! @brief Initialize BigClown CO2 Sensor

void bc_co2_sensor_init(bc_co2_sensor_t *self, const bc_co2_sensor_driver_t *driver);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_co2_sensor_set_event_handler(bc_co2_sensor_t *self, void (*event_handler)(bc_co2_sensor_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_co2_sensor_set_update_interval(bc_co2_sensor_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return False When other measurement is in progress

bool bc_co2_sensor_measure(bc_co2_sensor_t *self);

//! @brief Get CO2 concentration in ppm (parts per million)
//! @param[in] self Instance
//! @param[out] ppm Pointer to variable where result will be stored
//! @return true On success
//! @return false On failure

bool bc_co2_sensor_get_concentration_ppm(bc_co2_sensor_t *self, float *ppm);

//! @brief Request sensor calibration
//! @param[in] self Instance
//! @param[in] calibration Calibration type

void bc_co2_sensor_calibration(bc_co2_sensor_t *self, bc_co2_sensor_calibration_t calibration);

//! @}

#endif // _BC_CO2_SENSOR_H
