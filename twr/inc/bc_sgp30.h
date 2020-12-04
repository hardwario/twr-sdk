#ifndef _BC_SGP30_H
#define _BC_SGP30_H

#include <bc_i2c.h>
#include <bc_scheduler.h>

//! @addtogroup bc_sgp30 bc_sgp30
//! @brief Driver for SGP30 VOC gas sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_SGP30_EVENT_ERROR = 0,

    //! @brief Update event
    BC_SGP30_EVENT_UPDATE = 1

} bc_sgp30_event_t;

//! @brief SGP30 instance

typedef struct bc_sgp30_t bc_sgp30_t;

//! @cond

typedef enum
{
    BC_SGP30_STATE_ERROR = -1,
    BC_SGP30_STATE_INITIALIZE = 0,
    BC_SGP30_STATE_GET_FEATURE_SET = 1,
    BC_SGP30_STATE_READ_FEATURE_SET = 2,
    BC_SGP30_STATE_INIT_AIR_QUALITY = 3,
    BC_SGP30_STATE_SET_HUMIDITY = 4,
    BC_SGP30_STATE_MEASURE_AIR_QUALITY = 5,
    BC_SGP30_STATE_READ_AIR_QUALITY = 6

} bc_sgp30_state_t;

struct bc_sgp30_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(bc_sgp30_t *, bc_sgp30_event_t, void *);
    void *_event_param;
    bc_tick_t _update_interval;
    bc_sgp30_state_t _state;
    bc_tick_t _tick_ready;
    bc_tick_t _tick_last_measurement;
    bool _hit_error;
    bool _measurement_valid;
    uint16_t _co2eq;
    uint16_t _tvoc;
    uint16_t _ah_scaled;
};

//! @endcond

//! @brief Initialize SGP30
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void bc_sgp30_init(bc_sgp30_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_sgp30_set_event_handler(bc_sgp30_t *self, void (*event_handler)(bc_sgp30_t *, bc_sgp30_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_sgp30_set_update_interval(bc_sgp30_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_sgp30_measure(bc_sgp30_t *self);

//! @brief Get measured CO2eq in ppm (parts per million)
//! @param[in] self Instance
//! @param[out] ppm Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_sgp30_get_co2eq_ppm(bc_sgp30_t *self, uint16_t *ppm);

//! @brief Get measured TVOC in ppb (parts per billion)
//! @param[in] self Instance
//! @param[out] ppb Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_sgp30_get_tvoc_ppb(bc_sgp30_t *self, uint16_t *ppb);

//! @brief Set sensor compensation (absolute humidity is calculated from temperature and relative humidity)
//! @param[in] self Instance
//! @param[in] t_celsius Pointer to variable holding temperature in degrees of celsius (must be NULL if not available)
//! @param[in] rh_percentage Pointer to variable holding relative humidity in percentage (must be NULL if not available)
//! @return Absolute humidity in grams per cubic meter

float bc_sgp30_set_compensation(bc_sgp30_t *self, float *t_celsius, float *rh_percentage);

//! @}

#endif // _BC_SGP30_H
