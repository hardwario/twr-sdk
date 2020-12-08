#ifndef _TWR_MODULE_CLIMATE_H
#define _TWR_MODULE_CLIMATE_H

#include <twr_tick.h>

//! @addtogroup twr_module_climate twr_module_climate
//! @brief Driver for HARDWARIO Climate Module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event for thermometer
    TWR_MODULE_CLIMATE_EVENT_ERROR_THERMOMETER = 0,

    //! @brief Error event for hygrometer
    TWR_MODULE_CLIMATE_EVENT_ERROR_HYGROMETER = 1,

    //! @brief Error event for lux meter
    TWR_MODULE_CLIMATE_EVENT_ERROR_LUX_METER = 2,

    //! @brief Error event for barometer
    TWR_MODULE_CLIMATE_EVENT_ERROR_BAROMETER = 3,

    //! @brief Update event for thermometer
    TWR_MODULE_CLIMATE_EVENT_UPDATE_THERMOMETER = 4,

    //! @brief Update event for hygrometer
    TWR_MODULE_CLIMATE_EVENT_UPDATE_HYGROMETER = 5,

    //! @brief Update event for lux meter
    TWR_MODULE_CLIMATE_EVENT_UPDATE_LUX_METER = 6,

    //! @brief Update event for barometer
    TWR_MODULE_CLIMATE_EVENT_UPDATE_BAROMETER = 7

} twr_module_climate_event_t;

//! @brief Climate Module hardware revision

typedef enum
{
    //! @brief Hardware revision R1
    TWR_MODULE_CLIMATE_REVISION_R1 = 0,

    //! @brief Hardware revision R2
    TWR_MODULE_CLIMATE_REVISION_R2 = 1

} twr_module_climate_revision_t;

//! @brief Initialize HARDWARIO Climate Module

void twr_module_climate_init(void);

//! @brief Set callback function
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_module_climate_set_event_handler(void (*event_handler)(twr_module_climate_event_t, void *), void *event_param);

//! @brief Set measurement interval for all sensors
//! @param[in] interval Measurement interval

void twr_module_climate_set_update_interval_all_sensors(twr_tick_t interval);

//! @brief Set measurement interval for thermometer
//! @param[in] interval Measurement interval

void twr_module_climate_set_update_interval_thermometer(twr_tick_t interval);

//! @brief Set measurement interval for hygrometer
//! @param[in] interval Measurement interval

void twr_module_climate_set_update_interval_hygrometer(twr_tick_t interval);

//! @brief Set measurement interval for lux meter
//! @param[in] interval Measurement interval

void twr_module_climate_set_update_interval_lux_meter(twr_tick_t interval);

//! @brief Set measurement interval for barometer
//! @param[in] interval Measurement interval

void twr_module_climate_set_update_interval_barometer(twr_tick_t interval);

//! @brief Start measurement of all sensors manually
//! @return true On success
//! @return false When other measurement is in progress

bool twr_module_climate_measure_all_sensors(void);

//! @brief Start thermometer measurement manually
//! @return true On success
//! @return false When other measurement is in progress

bool twr_module_climate_measure_thermometer(void);

//! @brief Start hygrometer measurement manually
//! @return true On success
//! @return false When other measurement is in progress

bool twr_module_climate_measure_hygrometer(void);

//! @brief Start lux meter measurement manually
//! @return true On success
//! @return false When other measurement is in progress

bool twr_module_climate_measure_lux_meter(void);

//! @brief Start barometer measurement manually
//! @return true On success
//! @return false When other measurement is in progress

bool twr_module_climate_measure_barometer(void);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_module_climate_get_temperature_celsius(float *celsius);

//! @brief Get measured temperature in degrees of Fahrenheit
//! @param[in] fahrenheit Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_module_climate_get_temperature_fahrenheit(float *fahrenheit);

//! @brief Get measured temperature in kelvin
//! @param[in] kelvin Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_module_climate_get_temperature_kelvin(float *kelvin);

//! @brief Get measured humidity as percentage
//! @param[in] percentage Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_module_climate_get_humidity_percentage(float *percentage);

//! @brief Get measured illuminance in lux
//! @param[in] lux Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_module_climate_get_illuminance_lux(float *lux);

//! @brief Get measured altitude in meters
//! @param[in] meter Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_module_climate_get_altitude_meter(float *meter);

//! @brief Get measured pressure in Pascal
//! @param[in] pascal Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_module_climate_get_pressure_pascal(float *pascal);

//! @brief Get hardware revision
twr_module_climate_revision_t twr_module_climate_get_revision(void);

//! @}

#endif // _TWR_MODULE_CLIMATE_H
