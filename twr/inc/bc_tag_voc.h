#ifndef _TWR_TAG_VOC_H
#define _TWR_TAG_VOC_H

#include <twr_sgp30.h>

//! @addtogroup twr_tag_voc twr_tag_voc
//! @brief Driver for HARDWARIO VOC Module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_TAG_VOC_EVENT_ERROR = TWR_SGP30_EVENT_ERROR,

    //! @brief Update event
    TWR_TAG_VOC_EVENT_UPDATE = TWR_SGP30_EVENT_UPDATE

} twr_tag_voc_event_t;

//! @brief HARDWARIO VOC Module instance

typedef twr_sgp30_t twr_tag_voc_t;

//! @brief Initialize HARDWARIO VOC Module
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel

void twr_tag_voc_init(twr_tag_voc_t *self, twr_i2c_channel_t i2c_channel);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_tag_voc_set_event_handler(twr_tag_voc_t *self, void (*event_handler)(twr_tag_voc_t *, twr_tag_voc_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_tag_voc_set_update_interval(twr_tag_voc_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_tag_voc_measure(twr_tag_voc_t *self);

//! @brief Get measured CO2eq in ppm (parts per million)
//! @param[in] self Instance
//! @param[out] ppm Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tag_voc_get_co2eq_ppm(twr_tag_voc_t *self, uint16_t *ppm);

//! @brief Get measured TVOC in ppb (parts per billion)
//! @param[in] self Instance
//! @param[out] ppb Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tag_voc_get_tvoc_ppb(twr_tag_voc_t *self, uint16_t *ppb);

//! @brief Set sensor compensation (absolute humidity is calculated from temperature and relative humidity)
//! @param[in] self Instance
//! @param[in] t_celsius Pointer to variable holding temperature in degrees of celsius (must be NULL if not available)
//! @param[in] rh_percentage Pointer to variable holding relative humidity in percentage (must be NULL if not available)
//! @return Absolute humidity in grams per cubic meter

float twr_tag_voc_set_compensation(twr_tag_voc_t *self, float *t_celsius, float *rh_percentage);

//! @}

#endif // _TWR_TAG_VOC_H
