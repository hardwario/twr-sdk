#ifndef _TWR_TAG_BAROMETER_H
#define _TWR_TAG_BAROMETER_H

#include <twr_mpl3115a2.h>

//! @addtogroup twr_tag_barometer twr_tag_barometer
//! @brief Driver for HARDWARIO Barometer Module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_TAG_BAROMETER_EVENT_ERROR = TWR_MPL3115A2_EVENT_ERROR,

    //! @brief Update event
    TWR_TAG_BAROMETER_EVENT_UPDATE = TWR_MPL3115A2_EVENT_UPDATE

} twr_tag_barometer_event_t;

//! @brief HARDWARIO Barometer Module instance

typedef twr_mpl3115a2_t twr_tag_barometer_t;

//! @brief Initialize HARDWARIO Barometer Module
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel

void twr_tag_barometer_init(twr_tag_barometer_t *self, twr_i2c_channel_t i2c_channel);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_tag_barometer_set_event_handler(twr_tag_barometer_t *self, void (*event_handler)(twr_tag_barometer_t *, twr_tag_barometer_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_tag_barometer_set_update_interval(twr_tag_barometer_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_tag_barometer_measure(twr_tag_barometer_t *self);

//! @brief Get measured altitude in meters
//! @param[in] self Instance
//! @param[in] meter Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tag_barometer_get_altitude_meter(twr_tag_barometer_t *self, float *meter);

//! @brief Get measured pressure in Pascal
//! @param[in] self Instance
//! @param[in] pascal Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tag_barometer_get_pressure_pascal(twr_tag_barometer_t *self, float *pascal);

//! @}

#endif // _TWR_TAG_BAROMETER_H
