#ifndef _BC_TAG_VOC_H
#define _BC_TAG_VOC_H

#include <bc_sgp30.h>

//! @addtogroup bc_tag_voc bc_tag_voc
//! @brief Driver for BigClown VOC Tag
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_TAG_VOC_EVENT_ERROR = BC_SGP30_EVENT_ERROR,

    //! @brief Update event
    BC_TAG_VOC_EVENT_UPDATE = BC_SGP30_EVENT_UPDATE

} bc_tag_voc_event_t;

//! @brief BigClown VOC Tag instance

typedef bc_sgp30_t bc_tag_voc_t;

//! @brief Initialize BigClown VOC Tag
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel

void bc_tag_voc_init(bc_tag_voc_t *self, bc_i2c_channel_t i2c_channel);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_tag_voc_set_event_handler(bc_tag_voc_t *self, void (*event_handler)(bc_tag_voc_t *, bc_tag_voc_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_tag_voc_set_update_interval(bc_tag_voc_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_tag_voc_measure(bc_tag_voc_t *self);

//! @brief Get measured CO2eq in ppm (parts per million)
//! @param[in] self Instance
//! @param[out] ppm Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tag_voc_get_co2eq_ppm(bc_tag_voc_t *self, uint16_t *ppm);

//! @brief Get measured TVOC in ppb (parts per billion)
//! @param[in] self Instance
//! @param[out] ppb Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_tag_voc_get_tvoc_ppb(bc_tag_voc_t *self, uint16_t *ppb);

//! @brief Set sensor compensation (absolute humidity is calculated from temperature and relative humidity)
//! @param[in] self Instance
//! @param[in] t_celsius Pointer to variable holding temperature in degrees of celsius (must be NULL if not available)
//! @param[in] rh_percentage Pointer to variable holding relative humidity in percentage (must be NULL if not available)
//! @return Absolute humidity in grams per cubic meter

float bc_tag_voc_set_compensation(bc_tag_voc_t *self, float *t_celsius, float *rh_percentage);

//! @}

#endif // _BC_TAG_VOC_H
