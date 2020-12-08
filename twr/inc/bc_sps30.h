#ifndef _TWR_SPS30_H
#define _TWR_SPS30_H

#include <twr_i2c.h>
#include <twr_scheduler.h>

//! @addtogroup twr_sps30 twr_sps30
//! @brief Driver for SPS30 PM sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_SPS30_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_SPS30_EVENT_UPDATE = 1

} twr_sps30_event_t;

//! @brief SPS30 instance

typedef struct twr_sps30_t twr_sps30_t;

//! @cond

typedef enum
{
    TWR_SPS30_STATE_ERROR = -1,
    TWR_SPS30_STATE_READY = 0,
    TWR_SPS30_STATE_INITIALIZE = 1,
    TWR_SPS30_STATE_GET_SERIAL_NUMBER = 2,
    TWR_SPS30_STATE_READ_SERIAL_NUMBER = 3,
    TWR_SPS30_STATE_START_MEASUREMENT = 4,
    TWR_SPS30_STATE_SET_DATAREADY_FLAG = 5,
    TWR_SPS30_STATE_READ_DATAREADY_FLAG = 6,
    TWR_SPS30_STATE_GET_MEASUREMENT_DATA = 7,
    TWR_SPS30_STATE_READ_MEASUREMENT_DATA = 8,
    TWR_SPS30_STATE_STOP_MEASUREMENT = 9

} twr_sps30_state_t;

//! @brief Mass concentration structure

typedef struct
{
    //! @brief Mass concentration PM1.0 in μg/m3
    float mc_1p0;

    //! @brief Mass concentration PM2.5 in μg/m3
    float mc_2p5;

    //! @brief Mass concentration PM4.0 in μg/m3
    float mc_4p0;

    //! @brief Mass concentration PM10 in μg/m3
    float mc_10p0;

} twr_sps30_mass_concentration_t;

//! @brief Number concentration structure

typedef struct
{
    //! @brief Number concentration PM0.5 in #/cm3
    float nc_0p5;

    //! @brief Number concentration PM1.0 in #/cm3
    float nc_1p0;

    //! @brief Number concentration PM2.5 in #/cm3
    float nc_2p5;

    //! @brief Number concentration PM4.0 in #/cm3
    float nc_4p0;

    //! @brief Number concentration PM10 in #/cm3
    float nc_10p0;

} twr_sps30_number_concentration_t;

struct twr_sps30_t
{
    twr_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    twr_scheduler_task_id_t _task_id_interval;
    twr_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(twr_sps30_t *, twr_sps30_event_t, void *);
    void *_event_param;
    twr_tick_t _update_interval;
    twr_sps30_state_t _state;
    bool _measurement_valid;
    twr_sps30_mass_concentration_t _mass_concentration;
    twr_sps30_number_concentration_t _number_concentration;
    float _typical_particle_size;
    twr_tick_t _startup_time;
    twr_tick_t _start_time;
};

//! @endcond

//! @brief Initialize SPS30
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void twr_sps30_init(twr_sps30_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_sps30_set_event_handler(twr_sps30_t *self, void (*event_handler)(twr_sps30_t *, twr_sps30_event_t, void *), void *event_param);

//! @brief Set startup time (how long the fan blows air before the measurement)
//! @param[in] self Instance
//! @param[in] interval Startup time

void twr_sps30_set_startup_time(twr_sps30_t *self, twr_tick_t startup_time);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_sps30_set_update_interval(twr_sps30_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_sps30_measure(twr_sps30_t *self);

//! @brief Get measured mass concentration in μg/m3
//! @param[in] self Instance
//! @param[out] mass_concentration Pointer to mass concentration structure
//! @return true When value is valid
//! @return false When value is invalid

bool twr_sps30_get_mass_concentration(twr_sps30_t *self, twr_sps30_mass_concentration_t *mass_concentration);

//! @brief Get measured number concentration in #/cm3
//! @param[in] self Instance
//! @param[out] number_concentration Pointer to number concentration structure
//! @return true When value is valid
//! @return false When value is invalid

bool twr_sps30_get_number_concentration(twr_sps30_t *self, twr_sps30_number_concentration_t *number_concentration);

//! @brief Get measured typical particle size in μm
//! @param[in] self Instance
//! @param[out] typical_particle_size Pointer to typical particle size
//! @return true When value is valid
//! @return false When value is invalid

bool twr_sps30_get_typical_particle_size(twr_sps30_t *self, float *typical_particle_size);

//! @}

#endif // _TWR_SPS30_H
