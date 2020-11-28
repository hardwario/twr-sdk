#ifndef _BC_LIS2DH12_H
#define _BC_LIS2DH12_H

#include <bc_i2c.h>
#include <bc_tick.h>
#include <bc_scheduler.h>

//! @addtogroup bc_lis2dh12 bc_lis2dh12
//! @brief Driver for LIS2DH12 3-axis MEMS accelerometer
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_LIS2DH12_EVENT_ERROR = 0,

    //! @brief Update event
    BC_LIS2DH12_EVENT_UPDATE = 1,

    //! @brief Alarm event
    BC_LIS2DH12_EVENT_ALARM = 2

} bc_lis2dh12_event_t;

//! @brief Resolution and mode

typedef enum
{
    //! @brief 10-bit data output (Normal mode) (Default)
    BC_LIS2DH12_RESOLUTION_10BIT = 0,

    //! @brief 12-bit data output (High-resolution mode)
    BC_LIS2DH12_RESOLUTION_12BIT = 1,

    //! @brief 8-bit data output (Low-power mode)
    BC_LIS2DH12_RESOLUTION_8BIT = 2

} bc_lis2dh12_resolution_t;

typedef enum
{
    //! @brief ±2 g (Default)
    BC_LIS2DH12_SCALE_2G = 0,

    //! @brief ±4 g
    BC_LIS2DH12_SCALE_4G = 1,

    //! @brief ±8 g
    BC_LIS2DH12_SCALE_8G = 2,

    //! @brief ±16 g
    BC_LIS2DH12_SCALE_16G = 3,

} bc_lis2dh12_scale_t;

//! @brief LIS2DH12 result in raw values

typedef struct
{
    //! @brief X-axis
    int16_t x_axis;

    //! @brief Y-axis
    int16_t y_axis;

    //! @brief Z-axis
    int16_t z_axis;

} bc_lis2dh12_result_raw_t;

//! @brief LIS2DH12 result in g

typedef struct
{
    //! @brief X-axis
    float x_axis;

    //! @brief Y-axis
    float y_axis;

    //! @brief Z-axis
    float z_axis;

} bc_lis2dh12_result_g_t;

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

} bc_lis2dh12_alarm_t;

//! @brief LIS2DH12 instance

typedef struct bc_lis2dh12_t bc_lis2dh12_t;

//! @cond

typedef enum
{
    BC_LIS2DH12_STATE_ERROR = -1,
    BC_LIS2DH12_STATE_INITIALIZE = 0,
    BC_LIS2DH12_STATE_MEASURE = 1,
    BC_LIS2DH12_STATE_READ = 2,
    BC_LIS2DH12_STATE_UPDATE = 3

} bc_lis2dh12_state_t;

struct bc_lis2dh12_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    void (*_event_handler)(bc_lis2dh12_t *, bc_lis2dh12_event_t, void *);
    void *_event_param;
    bc_tick_t _update_interval;
    bc_lis2dh12_state_t _state;
    bool _accelerometer_valid;
    bc_lis2dh12_result_raw_t _raw;
    bool _alarm_active;
    bool _irq_flag;
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_measure;
    bool _measurement_active;
    bc_lis2dh12_resolution_t _resolution;
    bc_lis2dh12_scale_t _scale;
};

//! @endcond

//! @brief Initialize LIS2DH12
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

bool bc_lis2dh12_init(bc_lis2dh12_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_lis2dh12_set_event_handler(bc_lis2dh12_t *self, void (*event_handler)(bc_lis2dh12_t *, bc_lis2dh12_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_lis2dh12_set_update_interval(bc_lis2dh12_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_lis2dh12_measure(bc_lis2dh12_t *self);

//! @brief Get measured acceleration as raw value
//! @param[in] self Instance
//! @param[in] result_raw Pointer to structure where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_lis2dh12_get_result_raw(bc_lis2dh12_t *self, bc_lis2dh12_result_raw_t *result_raw);

//! @brief Get measured acceleration in g
//! @param[in] self Instance
//! @param[in] result_g Pointer to structure where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_lis2dh12_get_result_g(bc_lis2dh12_t *self, bc_lis2dh12_result_g_t *result_g);

//! @brief Enable or disable accelerometer threshold alarm
//! @param[in] self Instance
//! @param[in] alarm Pointer to structure with alarm configuration, if null then disable the alarm
//! @return true When configuration was successful
//! @return false When configuration was not successful

bool bc_lis2dh12_set_alarm(bc_lis2dh12_t *self, bc_lis2dh12_alarm_t *alarm);

//! @brief Set resolution
//! @param[in] self Instance
//! @param[in] resolution
//! @return true When configuration was successful
//! @return false When configuration was not successful

bool bc_lis2dh12_set_resolution(bc_lis2dh12_t *self, bc_lis2dh12_resolution_t resolution);

//! @brief Set scale
//! @param[in] self Instance
//! @param[in] scale
//! @return true When configuration was successful
//! @return false When configuration was not successful

bool bc_lis2dh12_set_scale(bc_lis2dh12_t *self, bc_lis2dh12_scale_t scale);

//! @}

#endif // _BC_LIS2DH12_H
