#ifndef BC_MODULE_GPS_H_
#define BC_MODULE_GPS_H_

#include <bc_sam_m8.h>

//! @addtogroup bc_module_gps bc_module_gps
//! @brief Driver for GPS module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_MODULE_GPS_EVENT_ERROR = BC_SAM_M8_EVENT_ERROR,

    //! @brief Start event
    BC_MODULE_GPS_EVENT_START = BC_SAM_M8_EVENT_START,

    //! @brief Stop event
    BC_MODULE_GPS_EVENT_STOP = BC_SAM_M8_EVENT_STOP,

    //! @brief Update event
    BC_MODULE_GPS_EVENT_UPDATE = BC_SAM_M8_EVENT_UPDATE,

    //! @brief Timeout event
    BC_MODULE_GPS_EVENT_TIMEOUT = BC_SAM_M8_EVENT_TIMEOUT

} bc_module_gps_event_t;

//! @cond

typedef bc_sam_m8_t bc_module_gps_t;

typedef bc_sam_m8_event_handler_t bc_module_gps_event_handler_t;

//! @endcond

//! @brief Initialize GPS driver
//! @param[in] self Instance

void bc_module_gps_init(bc_module_gps_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_module_gps_set_event_handler(bc_module_gps_t *self, bc_module_gps_event_handler_t event_handler, void *event_param);

//! @brief Start GPS
//! @param[in] self Instance
//! @param[in] milliseconds Timeout interval in milliseconds

void bc_module_gps_start(bc_module_gps_t *self, uint64_t milliseconds);

//! @brief Stop GPS
//! @param[in] self Instance

void bc_module_gps_stop(bc_module_gps_t *self);

//! @brief Get GPS position
//! @param[in] self Instance
//! @param[out] Latitude latitude
//! @param[out] Longitude longitude
//! @return true On success
//! @return false On failure

bool bc_module_gps_get_position(bc_module_gps_t *self, float *latitude, float *longitude);

#endif /* BC_MODULE_GPS_H_ */
