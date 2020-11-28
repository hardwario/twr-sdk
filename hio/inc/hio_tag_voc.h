#ifndef _HIO_TAG_VOC_H
#define _HIO_TAG_VOC_H

#include <hio_sgp30.h>

//! @addtogroup hio_tag_voc hio_tag_voc
//! @brief Driver for HARDWARIO VOC Module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_TAG_VOC_EVENT_ERROR = HIO_SGP30_EVENT_ERROR,

    //! @brief Update event
    HIO_TAG_VOC_EVENT_UPDATE = HIO_SGP30_EVENT_UPDATE

} hio_tag_voc_event_t;

//! @brief HARDWARIO VOC Module instance

typedef hio_sgp30_t hio_tag_voc_t;

//! @brief Initialize HARDWARIO VOC Module
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel

void hio_tag_voc_init(hio_tag_voc_t *self, hio_i2c_channel_t i2c_channel);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_tag_voc_set_event_handler(hio_tag_voc_t *self, void (*event_handler)(hio_tag_voc_t *, hio_tag_voc_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void hio_tag_voc_set_update_interval(hio_tag_voc_t *self, hio_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool hio_tag_voc_measure(hio_tag_voc_t *self);

//! @brief Get measured CO2eq in ppm (parts per million)
//! @param[in] self Instance
//! @param[out] ppm Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_tag_voc_get_co2eq_ppm(hio_tag_voc_t *self, uint16_t *ppm);

//! @brief Get measured TVOC in ppb (parts per billion)
//! @param[in] self Instance
//! @param[out] ppb Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_tag_voc_get_tvoc_ppb(hio_tag_voc_t *self, uint16_t *ppb);

//! @brief Set sensor compensation (absolute humidity is calculated from temperature and relative humidity)
//! @param[in] self Instance
//! @param[in] t_celsius Pointer to variable holding temperature in degrees of celsius (must be NULL if not available)
//! @param[in] rh_percentage Pointer to variable holding relative humidity in percentage (must be NULL if not available)
//! @return Absolute humidity in grams per cubic meter

float hio_tag_voc_set_compensation(hio_tag_voc_t *self, float *t_celsius, float *rh_percentage);

//! @}

#endif // _HIO_TAG_VOC_H
