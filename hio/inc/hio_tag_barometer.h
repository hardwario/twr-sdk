#ifndef _HIO_TAG_BAROMETER_H
#define _HIO_TAG_BAROMETER_H

#include <hio_mpl3115a2.h>

//! @addtogroup hio_tag_barometer hio_tag_barometer
//! @brief Driver for HARDWARIO Barometer Module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_TAG_BAROMETER_EVENT_ERROR = HIO_MPL3115A2_EVENT_ERROR,

    //! @brief Update event
    HIO_TAG_BAROMETER_EVENT_UPDATE = HIO_MPL3115A2_EVENT_UPDATE

} hio_tag_barometer_event_t;

//! @brief HARDWARIO Barometer Module instance

typedef hio_mpl3115a2_t hio_tag_barometer_t;

//! @brief Initialize HARDWARIO Barometer Module
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel

void hio_tag_barometer_init(hio_tag_barometer_t *self, hio_i2c_channel_t i2c_channel);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_tag_barometer_set_event_handler(hio_tag_barometer_t *self, void (*event_handler)(hio_tag_barometer_t *, hio_tag_barometer_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void hio_tag_barometer_set_update_interval(hio_tag_barometer_t *self, hio_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool hio_tag_barometer_measure(hio_tag_barometer_t *self);

//! @brief Get measured altitude in meters
//! @param[in] self Instance
//! @param[in] meter Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_tag_barometer_get_altitude_meter(hio_tag_barometer_t *self, float *meter);

//! @brief Get measured pressure in Pascal
//! @param[in] self Instance
//! @param[in] pascal Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_tag_barometer_get_pressure_pascal(hio_tag_barometer_t *self, float *pascal);

//! @}

#endif // _HIO_TAG_BAROMETER_H
