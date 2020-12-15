#ifndef _TWR_MPL3115A2_H
#define _TWR_MPL3115A2_H

#include <twr_i2c.h>
#include <twr_scheduler.h>

//! @addtogroup twr_mpl3115a2 twr_mpl3115a2
//! @brief Driver for MPL3115A2 pressure/altitude sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_MPL3115A2_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_MPL3115A2_EVENT_UPDATE = 1

} twr_mpl3115a2_event_t;

//! @brief MPL3115A2 instance

typedef struct twr_mpl3115a2_t twr_mpl3115a2_t;

//! @cond

typedef enum
{
    TWR_MPL3115A2_STATE_ERROR = -1,
    TWR_MPL3115A2_STATE_INITIALIZE = 0,
    TWR_MPL3115A2_STATE_MEASURE_ALTITUDE = 1,
    TWR_MPL3115A2_STATE_READ_ALTITUDE = 2,
    TWR_MPL3115A2_STATE_MEASURE_PRESSURE = 3,
    TWR_MPL3115A2_STATE_READ_PRESSURE = 4,
    TWR_MPL3115A2_STATE_UPDATE = 5

} twr_mpl3115a2_state_t;

struct twr_mpl3115a2_t
{
    twr_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    twr_scheduler_task_id_t _task_id_interval;
    twr_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(twr_mpl3115a2_t *, twr_mpl3115a2_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    twr_tick_t _update_interval;
    twr_mpl3115a2_state_t _state;
    twr_tick_t _tick_ready;
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

void twr_mpl3115a2_init(twr_mpl3115a2_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Deinitialize MPL3115A2
//! @param[in] self Instance

void twr_mpl3115a2_deinit(twr_mpl3115a2_t *self);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_mpl3115a2_set_event_handler(twr_mpl3115a2_t *self, void (*event_handler)(twr_mpl3115a2_t *, twr_mpl3115a2_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_mpl3115a2_set_update_interval(twr_mpl3115a2_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_mpl3115a2_measure(twr_mpl3115a2_t *self);

//! @brief Get measured altitude in meters
//! @param[in] self Instance
//! @param[in] meter Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_mpl3115a2_get_altitude_meter(twr_mpl3115a2_t *self, float *meter);

//! @brief Get measured pressured in Pascal
//! @param[in] self Instance
//! @param[in] pascal Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_mpl3115a2_get_pressure_pascal(twr_mpl3115a2_t *self, float *pascal);

//! @}

#endif // _TWR_MPL3115A2_H
