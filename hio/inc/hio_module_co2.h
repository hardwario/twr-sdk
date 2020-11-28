#ifndef _HIO_MODULE_CO2_H
#define _HIO_MODULE_CO2_H

#include <hio_tick.h>
#include <hio_lp8.h>

//! @addtogroup hio_module_co2 hio_module_co2
//! @brief Driver for HARDWARIO CO2 Module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_MODULE_CO2_EVENT_ERROR = HIO_LP8_EVENT_ERROR,

    //! @brief Update event
    HIO_MODULE_CO2_EVENT_UPDATE = HIO_LP8_EVENT_UPDATE

} hio_module_co2_event_t;

//! @brief Initialize HARDWARIO CO2 Module

void hio_module_co2_init(void);

//! @brief Set callback function
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_module_co2_set_event_handler(void (*event_handler)(hio_module_co2_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] interval Measurement interval

void hio_module_co2_set_update_interval(hio_tick_t interval);

//! @brief Start measurement manually
//! @return true On success
//! @return False When other measurement is in progress

bool hio_module_co2_measure(void);

//! @brief Get CO2 concentration in ppm (parts per million)
//! @param[out] ppm Pointer to variable where result will be stored
//! @return true On success
//! @return false On failure

bool hio_module_co2_get_concentration_ppm(float *ppm);

//! @brief Get last error code
//! @param[out] pointer to the variable where error code will be stored
//! @return true On success
//! @return false On failure

bool hio_module_co2_get_error(hio_lp8_error_t *error);

//! @brief Request sensor calibration
//! @param[in] calibration Calibration type

void hio_module_co2_calibration(hio_lp8_calibration_t calibration);

//! @}

#endif // _HIO_MODULE_CO2_H
