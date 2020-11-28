#ifndef _HIO_TAG_VOC_LP_H
#define _HIO_TAG_VOC_LP_H

#include <hio_sgpc3.h>

//! @addtogroup hio_tag_voc_lp hio_tag_voc_lp
//! @brief Driver for HARDWARIO VOC-LP Module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_TAG_VOC_LP_EVENT_ERROR = HIO_SGPC3_EVENT_ERROR,

    //! @brief Update event
    HIO_TAG_VOC_LP_EVENT_UPDATE = HIO_SGPC3_EVENT_UPDATE

} hio_tag_voc_lp_event_t;

//! @brief HARDWARIO VOC-LP Module instance

typedef hio_sgpc3_t hio_tag_voc_lp_t;

//! @brief Initialize HARDWARIO VOC-LP Module
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel

void hio_tag_voc_lp_init(hio_tag_voc_lp_t *self, hio_i2c_channel_t i2c_channel);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_tag_voc_lp_set_event_handler(hio_tag_voc_lp_t *self, void (*event_handler)(hio_tag_voc_lp_t *, hio_tag_voc_lp_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void hio_tag_voc_lp_set_update_interval(hio_tag_voc_lp_t *self, hio_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool hio_tag_voc_lp_measure(hio_tag_voc_lp_t *self);

//! @brief Get measured TVOC in ppb (parts per billion)
//! @param[in] self Instance
//! @param[out] ppb Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_tag_voc_lp_get_tvoc_ppb(hio_tag_voc_lp_t *self, uint16_t *ppb);

//! @brief Set sensor compensation (absolute humidity is calculated from temperature and relative humidity)
//! @param[in] self Instance
//! @param[in] t_celsius Pointer to variable holding temperature in degrees of celsius (must be NULL if not available)
//! @param[in] rh_percentage Pointer to variable holding relative humidity in percentage (must be NULL if not available)
//! @return Absolute humidity in grams per cubic meter

float hio_tag_voc_lp_set_compensation(hio_tag_voc_lp_t *self, float *t_celsius, float *rh_percentage);

//! @}

#endif // _HIO_TAG_VOC_LP_H
