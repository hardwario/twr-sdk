#ifndef _TWR_LIS2DH12_H
#define _TWR_LIS2DH12_H

#include <twr_i2c.h>
#include <twr_tick.h>
#include <twr_scheduler.h>

//! @addtogroup twr_lis2dh12 twr_lis2dh12
//! @brief Driver for LIS2DH12 3-axis MEMS accelerometer
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_LIS2DH12_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_LIS2DH12_EVENT_UPDATE = 1,

    //! @brief Alarm event
    TWR_LIS2DH12_EVENT_ALARM = 2

} twr_lis2dh12_event_t;

//! @brief Resolution and mode

typedef enum
{
    //! @brief 10-bit data output (Normal mode) (Default)
    TWR_LIS2DH12_RESOLUTION_10BIT = 0,

    //! @brief 12-bit data output (High-resolution mode)
    TWR_LIS2DH12_RESOLUTION_12BIT = 1,

    //! @brief 8-bit data output (Low-power mode)
    TWR_LIS2DH12_RESOLUTION_8BIT = 2

} twr_lis2dh12_resolution_t;

typedef enum
{
    //! @brief ±2 g (Default)
    TWR_LIS2DH12_SCALE_2G = 0,

    //! @brief ±4 g
    TWR_LIS2DH12_SCALE_4G = 1,

    //! @brief ±8 g
    TWR_LIS2DH12_SCALE_8G = 2,

    //! @brief ±16 g
    TWR_LIS2DH12_SCALE_16G = 3,

} twr_lis2dh12_scale_t;

//! @brief LIS2DH12 result in raw values

typedef struct
{
    //! @brief X-axis
    int16_t x_axis;

    //! @brief Y-axis
    int16_t y_axis;

    //! @brief Z-axis
    int16_t z_axis;

} twr_lis2dh12_result_raw_t;

//! @brief LIS2DH12 result in g

typedef struct
{
    //! @brief X-axis
    float x_axis;

    //! @brief Y-axis
    float y_axis;

    //! @brief Z-axis
    float z_axis;

} twr_lis2dh12_result_g_t;

//! @brief LIS2DH12 alarm set structure

typedef struct
{
    //! @brief Alarm threshold in g
    float threshold;
    uint8_t duration;
    bool x_low;
    bool x_high;
    bool y_low;
    bool y_high;
    bool z_low;
    bool z_high;

} twr_lis2dh12_alarm_t;

//! @brief LIS2DH12 instance

typedef struct twr_lis2dh12_t twr_lis2dh12_t;

//! @cond

typedef enum
{
    TWR_LIS2DH12_STATE_ERROR = -1,
    TWR_LIS2DH12_STATE_INITIALIZE = 0,
    TWR_LIS2DH12_STATE_MEASURE = 1,
    TWR_LIS2DH12_STATE_READ = 2,
    TWR_LIS2DH12_STATE_UPDATE = 3

} twr_lis2dh12_state_t;

struct twr_lis2dh12_t
{
    twr_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    void (*_event_handler)(twr_lis2dh12_t *, twr_lis2dh12_event_t, void *);
    void *_event_param;
    twr_tick_t _update_interval;
    twr_lis2dh12_state_t _state;
    bool _accelerometer_valid;
    twr_lis2dh12_result_raw_t _raw;
    bool _alarm_active;
    bool _irq_flag;
    twr_scheduler_task_id_t _task_id_interval;
    twr_scheduler_task_id_t _task_id_measure;
    bool _measurement_active;
    twr_lis2dh12_resolution_t _resolution;
    twr_lis2dh12_scale_t _scale;
};

//! @endcond

//! @brief Initialize LIS2DH12
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

bool twr_lis2dh12_init(twr_lis2dh12_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_lis2dh12_set_event_handler(twr_lis2dh12_t *self, void (*event_handler)(twr_lis2dh12_t *, twr_lis2dh12_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_lis2dh12_set_update_interval(twr_lis2dh12_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_lis2dh12_measure(twr_lis2dh12_t *self);

//! @brief Get measured acceleration as raw value
//! @param[in] self Instance
//! @param[in] result_raw Pointer to structure where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_lis2dh12_get_result_raw(twr_lis2dh12_t *self, twr_lis2dh12_result_raw_t *result_raw);

//! @brief Get measured acceleration in g
//! @param[in] self Instance
//! @param[in] result_g Pointer to structure where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_lis2dh12_get_result_g(twr_lis2dh12_t *self, twr_lis2dh12_result_g_t *result_g);

//! @brief Enable or disable accelerometer threshold alarm
//! @param[in] self Instance
//! @param[in] alarm Pointer to structure with alarm configuration, if null then disable the alarm
//! @return true When configuration was successful
//! @return false When configuration was not successful

bool twr_lis2dh12_set_alarm(twr_lis2dh12_t *self, twr_lis2dh12_alarm_t *alarm);

//! @brief Set resolution
//! @param[in] self Instance
//! @param[in] resolution
//! @return true When configuration was successful
//! @return false When configuration was not successful

bool twr_lis2dh12_set_resolution(twr_lis2dh12_t *self, twr_lis2dh12_resolution_t resolution);

//! @brief Set scale
//! @param[in] self Instance
//! @param[in] scale
//! @return true When configuration was successful
//! @return false When configuration was not successful

bool twr_lis2dh12_set_scale(twr_lis2dh12_t *self, twr_lis2dh12_scale_t scale);

//! @}

#endif // _TWR_LIS2DH12_H
