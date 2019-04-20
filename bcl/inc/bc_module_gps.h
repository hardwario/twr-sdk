#ifndef _BC_MODULE_GPS_H
#define _BC_MODULE_GPS_H

#include <bc_led.h>
#include <bc_sam_m8q.h>

//! @addtogroup bc_module_gps bc_module_gps
//! @brief Driver for BigClown GPS Module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_MODULE_GPS_EVENT_ERROR = 0,

    //! @brief Start event
    BC_MODULE_GPS_EVENT_START = 1,

    //! @brief Update event
    BC_MODULE_GPS_EVENT_UPDATE = 2,

    //! @brief Stop event
    BC_MODULE_GPS_EVENT_STOP = 3

} bc_module_gps_event_t;

//! @brief Virtual LED channels

typedef enum
{
    //! @brief LCD red LED channel
    BC_MODULE_GPS_LED_RED = 0,

    //! @brief LCD green LED channel
    BC_MODULE_GPS_LED_GREEN  = 1

} bc_module_gps_led_t;

//! @brief Time data structure

typedef bc_sam_m8q_time_t bc_module_gps_time_t;

//! @brief Position data structure

typedef bc_sam_m8q_position_t bc_module_gps_position_t;

//! @brief Altitude data structure

typedef bc_sam_m8q_altitude_t bc_module_gps_altitude_t;

//! @brief Quality data structure

typedef bc_sam_m8q_quality_t bc_module_gps_quality_t;

//! @brief Accuracy data structure

typedef bc_sam_m8q_accuracy_t bc_module_gps_accuracy_t;

//! @cond

typedef void (bc_module_gps_event_handler_t)(bc_module_gps_event_t, void *);

//! @endcond

//! @brief Initialize BigClown GPS Module
//! @return true On success
//! @return false On failure

bool bc_module_gps_init(void);

//! @brief Set callback function
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_module_gps_set_event_handler(bc_module_gps_event_handler_t event_handler, void *event_param);

//! @brief Start tracking

void bc_module_gps_start(void);

//! @brief Stop tracking

void bc_module_gps_stop(void);

//! @brief Invalidate navigation data

void bc_module_gps_invalidate(void);

//! @brief Get time
//! @param[out] time Time data structure
//! @return true On success
//! @return false On failure

bool bc_module_gps_get_time(bc_module_gps_time_t *time);

//! @brief Get position
//! @param[out] position Position data structure
//! @return true On success
//! @return false On failure

bool bc_module_gps_get_position(bc_module_gps_position_t *position);

//! @brief Get altitude
//! @param[out] altitude Altitude data structure
//! @return true On success
//! @return false On failure

bool bc_module_gps_get_altitude(bc_module_gps_altitude_t *altitude);

//! @brief Get quality
//! @param[out] quality Quality data structure
//! @return true On success
//! @return false On failure

bool bc_module_gps_get_quality(bc_module_gps_quality_t *quality);

//! @brief Get accuracy
//! @param[out] accuracy Accuracy data structure
//! @return true On success
//! @return false On failure

bool bc_module_gps_get_accuracy(bc_module_gps_accuracy_t *accuracy);

//! @brief Get LED driver
//! @return Driver for on-board LED

const bc_led_driver_t *bc_module_gps_get_led_driver(void);

#endif // _BC_MODULE_GPS_H
