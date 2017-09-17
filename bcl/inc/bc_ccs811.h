#ifndef _BC_CCS811_H
#define _BC_CCS811_H

#include <bcl.h>

//! @addtogroup bc_ccs811 bc_ccs811
//! @brief Driver for CCS811 air quality sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_CCS811_EVENT_ERROR = 0,

    //! @brief Update event
    BC_CCS811_EVENT_UPDATE = 1

} bc_ccs811_event_t;

//! @brief CCS811 instance

typedef struct bc_ccs811_t bc_ccs811_t;

//! @cond

typedef enum
{
    BC_CCS811_STATE_ERROR = -1,
    BC_CCS811_STATE_RESTART = 0,
    BC_CCS811_STATE_INITIALIZE = 1,
    BC_CCS811_STATE_MEAS_START = 2,
    BC_CCS811_STATE_MEAS = 3,
    BC_CCS811_STATE_READ = 4,
    BC_CCS811_STATE_UPDATE = 5

} bc_ccs811_state_t;

struct bc_ccs811_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(bc_ccs811_t *, bc_ccs811_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    bc_tick_t _update_interval;
    bc_ccs811_state_t _state;
    uint16_t _baseline;
    bool _co2_valid;
    uint16_t _reg_co2;
};

//! @endcond

//! @brief Initialize CCS811
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void bc_ccs811_init(bc_ccs811_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_ccs811_set_event_handler(bc_ccs811_t *self, void (*event_handler)(bc_ccs811_t *, bc_ccs811_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_ccs811_set_update_interval(bc_ccs811_t *self, bc_tick_t interval);

//! @brief Get baseline
//! @param[in] self Instance
//! @param[in] baseline Actual baseline

void bc_ccs811_get_baseline(bc_ccs811_t *self, uint16_t *baseline);

//! @brief Set baseline
//! @param[in] self Instance
//! @param[in] baseline Desired baseline

void bc_ccs811_set_baseline(bc_ccs811_t *self, uint16_t baseline);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_ccs811_measure(bc_ccs811_t *self);

//! @brief Get measured co2 in ppm
//! @param[in] self Instance
//! @param[in] ppm Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_ccs811_get_co2_ppm(bc_ccs811_t *self, float *ppm);

//! @}

#endif /* _INC_BC_CCS811_H */
