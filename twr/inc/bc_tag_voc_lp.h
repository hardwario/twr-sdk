#ifndef _TWR_TAG_VOC_LP_H
#define _TWR_TAG_VOC_LP_H

#include <twr_sgpc3.h>

//! @addtogroup twr_tag_voc_lp twr_tag_voc_lp
//! @brief Driver for HARDWARIO VOC-LP Module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_TAG_VOC_LP_EVENT_ERROR = TWR_SGPC3_EVENT_ERROR,

    //! @brief Update event
    TWR_TAG_VOC_LP_EVENT_UPDATE = TWR_SGPC3_EVENT_UPDATE

} twr_tag_voc_lp_event_t;

//! @brief HARDWARIO VOC-LP Module instance

typedef twr_sgpc3_t twr_tag_voc_lp_t;

//! @brief Initialize HARDWARIO VOC-LP Module
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel

void twr_tag_voc_lp_init(twr_tag_voc_lp_t *self, twr_i2c_channel_t i2c_channel);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_tag_voc_lp_set_event_handler(twr_tag_voc_lp_t *self, void (*event_handler)(twr_tag_voc_lp_t *, twr_tag_voc_lp_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_tag_voc_lp_set_update_interval(twr_tag_voc_lp_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_tag_voc_lp_measure(twr_tag_voc_lp_t *self);

//! @brief Get measured TVOC in ppb (parts per billion)
//! @param[in] self Instance
//! @param[out] ppb Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_tag_voc_lp_get_tvoc_ppb(twr_tag_voc_lp_t *self, uint16_t *ppb);

//! @brief Set sensor compensation (absolute humidity is calculated from temperature and relative humidity)
//! @param[in] self Instance
//! @param[in] t_celsius Pointer to variable holding temperature in degrees of celsius (must be NULL if not available)
//! @param[in] rh_percentage Pointer to variable holding relative humidity in percentage (must be NULL if not available)
//! @return Absolute humidity in grams per cubic meter

float twr_tag_voc_lp_set_compensation(twr_tag_voc_lp_t *self, float *t_celsius, float *rh_percentage);

//! @}

#endif // _TWR_TAG_VOC_LP_H
