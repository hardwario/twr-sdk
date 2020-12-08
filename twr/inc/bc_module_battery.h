#ifndef _TWR_MODULE_BATTERY_H
#define _TWR_MODULE_BATTERY_H

#include <twr_tick.h>

//! @addtogroup twr_module_battery twr_module_battery
//! @brief Driver for Battery Module
//! @{

//! @brief Battery Module format

typedef enum
{
    //! @brief Format is unknown
    TWR_MODULE_BATTERY_FORMAT_UNKNOWN = 0,

    //! @brief Format is standard 4xAAA
    TWR_MODULE_BATTERY_FORMAT_STANDARD = 1,

    //! @brief Format is mini 2xAAA
    TWR_MODULE_BATTERY_FORMAT_MINI = 2

} twr_module_battery_format_t;

//! @brief Battery Module event

typedef enum
{
    //! @brief Event low level
    TWR_MODULE_BATTERY_EVENT_LEVEL_LOW = 0,

    //! @brief Event critical level
    TWR_MODULE_BATTERY_EVENT_LEVEL_CRITICAL = 1,

    //! @brief Event update
    TWR_MODULE_BATTERY_EVENT_UPDATE = 2,

    //! @brief Event error
    TWR_MODULE_BATTERY_EVENT_ERROR = 3

} twr_module_battery_event_t;

//! @brief Initialize Battery Module

void twr_module_battery_init(void);

//! @brief Set callback function
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_module_battery_set_event_handler(void (*event_handler)(twr_module_battery_event_t, void *), void *event_param);

//! @brief Set update interval
//! @param[in] interval Update interval

void twr_module_battery_set_update_interval(twr_tick_t interval);

//! @brief Set voltage levels
//! @param[in] level_low_threshold Voltage level considered as low
//! @param[in] level_critical_threshold Voltage level considered as critical

void twr_module_battery_set_threshold_levels(float level_low_threshold, float level_critical_threshold);

//! @brief Get Battery Module format

twr_module_battery_format_t twr_module_battery_get_format();

//! @brief Start mesurement
//! @return true On success
//! @return false On failure

bool twr_module_battery_measure(void);

//! @brief Get Battery Module voltage
//! @param[out] voltage Measured voltage
//! @return true On success
//! @return false On failure

bool twr_module_battery_get_voltage(float *voltage);

//! @brief Get Battery Module charge in percents
//! @param[out] percentage Measured charge
//! @return true On success
//! @return false On failure

bool twr_module_battery_get_charge_level(int *percentage);

//! @brief Get Battery Module is pressent, can use without twr_module_battery_init

bool twr_module_battery_is_present(void);

//! @}

#endif // _TWR_MODULE_BATTERY_H
