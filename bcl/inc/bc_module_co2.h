#ifndef _BC_MODULE_CO2_H
#define _BC_MODULE_CO2_H

#include <bc_tick.h>

#define BC_MODULE_CO2_I2C_GPIO_EXPANDER_ADDRESS 0x38
#define BC_MODULE_CO2_I2C_UART_ADDRESS          0x4D

//! @addtogroup bc_module_co2 bc_module_co2
//! @brief Driver for BigClown CO2 Module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_MODULE_CO2_EVENT_ERROR = 0,

    //! @brief Update event
    BC_MODULE_CO2_EVENT_UPDATE = 1

} bc_module_co2_event_t;


//! @brief Calibration

typedef enum
{
    //! @brief Background calibration using unfiltered data
    BC_MODULE_CO2_CALIBRATION_BACKGROUND_UNFILTERED = 0x50,

    //! @brief Background calibration using filtered data
    BC_MODULE_CO2_CALIBRATION_BACKGROUND_FILTERED = 0x51,

    //! @brief Background calibration using unfiltered data + reset filters
    BC_MODULE_CO2_CALIBRATION_BACKGROUND_UNFILTERED_RF = 0x52,

    //! @brief Background calibration using filtered data + reset filters
    BC_MODULE_CO2_CALIBRATION_BACKGROUND_FILTERED_RF = 0x53,

    //! @brief ABC (based on filtered data)
    BC_MODULE_CO2_CALIBRATION_ABC = 0x70,

    //! @brief ABC (based on filtered data)  + reset filters
    BC_MODULE_CO2_CALIBRATION_ABC_RF = 0x72,

} bc_module_co2_calibration_t;

//! @brief Initialize BigClown CO2 Module

void bc_module_co2_init(void);

//! @brief Set callback function
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_module_co2_set_event_handler(void (*event_handler)(bc_module_co2_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] interval Measurement interval

void bc_module_co2_set_update_interval(bc_tick_t interval);

//! @brief Start measurement manually
//! @return true On success
//! @return False When other measurement is in progress

bool bc_module_co2_measure(void);

//! @brief Get co2 concentration
//! @param[out] concentration in ppm
//! @return true on success
//! @return false on failure

bool bc_module_co2_get_concentration(float *concentration);

//! @brief Set calibration request
//! @param[in] calibration type

void bc_module_co2_calibration(bc_module_co2_calibration_t calibration);

//! @}

#endif // _BC_MODULE_CO2_H
