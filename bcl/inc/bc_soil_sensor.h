#ifndef _BC_SOIL_SENSOR_H
#define _BC_SOIL_SENSOR_H

#include <bc_ds28e17.h>
#include <bc_tmp112.h>
#include <bc_zssc3123.h>

//! @addtogroup bc_soil_sensor bc_soil_sensor
//! @brief Driver for soil sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event for thermometer
    BC_SOIL_SENSOR_EVENT_ERROR_THERMOMETER = 0,

    //! @brief Error event for moisture
    BC_SOIL_SENSOR_EVENT_ERROR_MOISTURE = 1,

    //! @brief Update event for thermometer
    BC_SOIL_SENSOR_EVENT_UPDATE_THERMOMETER = 2,

    //! @brief Update event for moisture
    BC_SOIL_SENSOR_EVENT_UPDATE_MOISTURE = 3,

} bc_soil_sensor_event_t;

//! @brief Soil sensor instance

typedef struct bc_soil_sensor_t bc_soil_sensor_t;

struct bc_soil_sensor_t
{
    uint64_t _device_number;
    bc_ds28e17_t _ds28e17;
    bc_tmp112_t _tmp112;
    bc_zssc3123_t _zssc3123;
    void (*_event_handler)(bc_soil_sensor_t *, bc_soil_sensor_event_t, void *);
    void *_event_param;
};

//! @brief Initialize Soil sensor
//! @param[in] self Instance
//! @param[in] device_number Device number

void bc_soil_sensor_init(bc_soil_sensor_t *self, uint64_t device_number);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_soil_sensor_set_event_handler(bc_soil_sensor_t *self, void (*event_handler)(bc_soil_sensor_t *, bc_soil_sensor_event_t, void *), void *event_param);

//! @brief Set measurement interval for all sensors
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_soil_sensor_set_update_interval_all_sensors(bc_soil_sensor_t *self, bc_tick_t interval);

//! @brief Set measurement interval for thermometer
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_soil_sensor_set_update_interval_thermometer(bc_soil_sensor_t *self, bc_tick_t interval);

//! @brief Set measurement interval for moisture sensor
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_soil_sensor_set_update_interval_moisture_sesor(bc_soil_sensor_t *self, bc_tick_t interval);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_soil_sensor_get_temperature_celsius(bc_soil_sensor_t *self, float *celsius);

//! @brief Get measured temperature in degrees of Fahrenheit
//! @param[in] self Instance
//! @param[in] fahrenheit Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_soil_sensor_get_temperature_fahrenheit(bc_soil_sensor_t *self, float *fahrenheit);

//! @brief Get measured temperature in kelvin
//! @param[in] self Instance
//! @param[in] kelvin Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_soil_sensor_get_temperature_kelvin(bc_soil_sensor_t *self, float *kelvin);

//! @brief Get measured moisture in percent
//! @param[in] self Instance
//! @param[in] moisture Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_soil_sensor_get_moisture(bc_soil_sensor_t *self, int *moisture);

//! @brief Get Device number
//! @param[in] self Instance

uint64_t bc_soil_sensor_get_device_number(bc_soil_sensor_t *self);

//! @}

#endif // _BC_SOIL_SENSOR_H
