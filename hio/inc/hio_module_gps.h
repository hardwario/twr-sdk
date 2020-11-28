#ifndef _HIO_MODULE_GPS_H
#define _HIO_MODULE_GPS_H

#include <hio_led.h>
#include <hio_sam_m8q.h>

//! @addtogroup hio_module_gps hio_module_gps
//! @brief Driver for HARDWARIO GPS Module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_MODULE_GPS_EVENT_ERROR = 0,

    //! @brief Start event
    HIO_MODULE_GPS_EVENT_START = 1,

    //! @brief Update event
    HIO_MODULE_GPS_EVENT_UPDATE = 2,

    //! @brief Stop event
    HIO_MODULE_GPS_EVENT_STOP = 3

} hio_module_gps_event_t;

//! @brief Virtual LED channels

typedef enum
{
    //! @brief LCD red LED channel
    HIO_MODULE_GPS_LED_RED = 0,

    //! @brief LCD green LED channel
    HIO_MODULE_GPS_LED_GREEN  = 1

} hio_module_gps_led_t;

//! @brief Time data structure

typedef hio_sam_m8q_time_t hio_module_gps_time_t;

//! @brief Position data structure

typedef hio_sam_m8q_position_t hio_module_gps_position_t;

//! @brief Altitude data structure

typedef hio_sam_m8q_altitude_t hio_module_gps_altitude_t;

//! @brief Quality data structure

typedef hio_sam_m8q_quality_t hio_module_gps_quality_t;

//! @brief Accuracy data structure

typedef hio_sam_m8q_accuracy_t hio_module_gps_accuracy_t;

//! @cond

typedef void (hio_module_gps_event_handler_t)(hio_module_gps_event_t, void *);

//! @endcond

//! @brief Initialize HARDWARIO GPS Module
//! @return true On success
//! @return false On failure

bool hio_module_gps_init(void);

//! @brief Set callback function
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_module_gps_set_event_handler(hio_module_gps_event_handler_t event_handler, void *event_param);

//! @brief Start tracking

void hio_module_gps_start(void);

//! @brief Stop tracking

void hio_module_gps_stop(void);

//! @brief Invalidate navigation data

void hio_module_gps_invalidate(void);

//! @brief Get time
//! @param[out] time Time data structure
//! @return true On success
//! @return false On failure

bool hio_module_gps_get_time(hio_module_gps_time_t *time);

//! @brief Get position
//! @param[out] position Position data structure
//! @return true On success
//! @return false On failure

bool hio_module_gps_get_position(hio_module_gps_position_t *position);

//! @brief Get altitude
//! @param[out] altitude Altitude data structure
//! @return true On success
//! @return false On failure

bool hio_module_gps_get_altitude(hio_module_gps_altitude_t *altitude);

//! @brief Get quality
//! @param[out] quality Quality data structure
//! @return true On success
//! @return false On failure

bool hio_module_gps_get_quality(hio_module_gps_quality_t *quality);

//! @brief Get accuracy
//! @param[out] accuracy Accuracy data structure
//! @return true On success
//! @return false On failure

bool hio_module_gps_get_accuracy(hio_module_gps_accuracy_t *accuracy);

//! @brief Get LED driver
//! @return Driver for on-board LED

const hio_led_driver_t *hio_module_gps_get_led_driver(void);

#endif // _HIO_MODULE_GPS_H
