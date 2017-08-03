#ifndef _BC_MODULE_BATTERY_H
#define _BC_MODULE_BATTERY_H

#include <bc_tick.h>

//! @addtogroup bc_module_battery bc_module_battery
//! @brief Driver for Battery Module
//! @{

//! @brief Battery Module format

typedef enum
{
    //! @brief Format is standard
    BC_MODULE_BATTERY_FORMAT_STANDARD = 0,

    //! @brief Format is mini
    BC_MODULE_BATTERY_FORMAT_MINI = 1

} bc_module_battery_format_t;

//! @brief Battery Module event

typedef enum
{
    //! @brief Event low level
    BC_MODULE_BATTERY_EVENT_LEVEL_LOW = 0,

    //! @brief Event critical level
    BC_MODULE_BATTERY_EVENT_LEVEL_CRITICAL = 1,

    //! @brief Event update
    BC_MODULE_BATTERY_EVENT_UPDATE = 2

} bc_module_battery_event_t;

//! @brief Initialize Battery Module
//! @param[in] format Battery Module format

void bc_module_battery_init(bc_module_battery_format_t format);

//! @brief Set callback function
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_module_battery_set_event_handler(void (*event_handler)(bc_module_battery_event_t, void *), void *event_param);

//! @brief Set update interval
//! @param[in] interval Update interval

void bc_module_battery_set_update_interval(bc_tick_t interval);

//! @brief Set voltage levels
//! @param[in] level_low_threshold Voltage level considered as low
//! @param[in] level_critical_threshold Voltage level considered as critical

void bc_module_battery_set_threshold_levels(float level_low_threshold, float level_critical_threshold);

//! @brief Start mesurement
//! @return true On success
//! @return false On failure

bool bc_module_battery_measure(void);

//! @brief Get Battery Module voltage
//! @param[out] voltage Measured voltage
//! @return true On success
//! @return false On failure

bool bc_module_battery_get_voltage(float *voltage);

//! @brief Get Battery Module charge in percents
//! @param[out] percentage Measured charge
//! @return true On success
//! @return false On failure

bool bc_module_battery_get_charge_level(int *percentage);


//! @}

#endif // _BC_MODULE_BATTERY_H
