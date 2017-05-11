#ifndef _BC_MPL3115A2_H
#define _BC_MPL3115A2_H

#include <bc_i2c.h>
#include <bc_scheduler.h>

//! @addtogroup bc_mpl3115a2 bc_mpl3115a2
//! @brief Driver for MPL3115A2 pressure/altitude sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_MPL3115A2_EVENT_ERROR = 0,

    //! @brief Update event
    BC_MPL3115A2_EVENT_UPDATE = 1

} bc_mpl3115a2_event_t;

//! @brief MPL3115A2 instance

typedef struct bc_mpl3115a2_t bc_mpl3115a2_t;

//! @cond

typedef enum
{
    BC_MPL3115A2_STATE_ERROR = -1,
    BC_MPL3115A2_STATE_INITIALIZE = 0,
    BC_MPL3115A2_STATE_MEASURE_ALTITUDE = 1,
    BC_MPL3115A2_STATE_READ_ALTITUDE = 2,
    BC_MPL3115A2_STATE_MEASURE_PRESSURE = 3,
    BC_MPL3115A2_STATE_READ_PRESSURE = 4,
    BC_MPL3115A2_STATE_UPDATE = 5

} bc_mpl3115a2_state_t;

struct bc_mpl3115a2_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(bc_mpl3115a2_t *, bc_mpl3115a2_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    bc_tick_t _update_interval;
    bc_mpl3115a2_state_t _state;
    bc_tick_t _tick_ready;
    bool _altitude_valid;
    bool _pressure_valid;
    uint8_t _reg_out_p_msb_altitude;
    uint8_t _reg_out_p_csb_altitude;
    uint8_t _reg_out_p_lsb_altitude;
    uint8_t _reg_out_t_msb_altitude;
    uint8_t _reg_out_t_lsb_altitude;
    uint8_t _reg_out_p_msb_pressure;
    uint8_t _reg_out_p_csb_pressure;
    uint8_t _reg_out_p_lsb_pressure;
    uint8_t _reg_out_t_msb_pressure;
    uint8_t _reg_out_t_lsb_pressure;
};

//! @endcond

//! @brief Initialize MPL3115A2
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void bc_mpl3115a2_init(bc_mpl3115a2_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_mpl3115a2_set_event_handler(bc_mpl3115a2_t *self, void (*event_handler)(bc_mpl3115a2_t *, bc_mpl3115a2_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_mpl3115a2_set_update_interval(bc_mpl3115a2_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_mpl3115a2_measure(bc_mpl3115a2_t *self);

//! @brief Get measured altitude in meters
//! @param[in] self Instance
//! @param[in] meter Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_mpl3115a2_get_altitude_meter(bc_mpl3115a2_t *self, float *meter);

//! @brief Get measured pressured in Pascal
//! @param[in] self Instance
//! @param[in] pascal Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_mpl3115a2_get_pressure_pascal(bc_mpl3115a2_t *self, float *pascal);

//! @}

#endif // _BC_MPL3115A2_H
