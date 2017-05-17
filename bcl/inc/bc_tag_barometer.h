#ifndef _BC_TAG_BAROMETER_H
#define _BC_TAG_BAROMETER_H

#include <bc_mpl3115a2.h>

//! @addtogroup bc_tag_barometer bc_tag_barometer
//! @brief Driver for BigClown Barometer Tag
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_TAG_BAROMETER_EVENT_ERROR = BC_MPL3115A2_EVENT_ERROR,

    //! @brief Update event
    BC_TAG_BAROMETER_EVENT_UPDATE = BC_MPL3115A2_EVENT_UPDATE

} bc_tag_barometer_event_t;

//! @brief BigClown Barometer Tag instance

typedef bc_mpl3115a2_t bc_tag_barometer_t;

//! @brief Initialize BigClown Barometer Tag
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel

void bc_tag_barometer_init(bc_tag_barometer_t *self, bc_i2c_channel_t i2c_channel);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_tag_barometer_set_event_handler(bc_tag_barometer_t *self, void (*event_handler)(bc_tag_barometer_t *, bc_tag_barometer_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_tag_barometer_set_update_interval(bc_tag_barometer_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_tag_barometer_measure(bc_tag_barometer_t *self);

//! @brief Get measured altitude in meters
//! @param[in] self Instance
//! @param[in] meter Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tag_barometer_get_altitude_meter(bc_tag_barometer_t *self, float *meter);

//! @brief Get measured pressure in Pascal
//! @param[in] self Instance
//! @param[in] pascal Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tag_barometer_get_pressure_pascal(bc_tag_barometer_t *self, float *pascal);

//! @}

#endif // _BC_TAG_BAROMETER_H
