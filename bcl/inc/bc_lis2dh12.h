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
    uint8_t _out_x_l;
    uint8_t _out_x_h;
    uint8_t _out_y_l;
    uint8_t _out_y_h;
    uint8_t _out_z_l;
    uint8_t _out_z_h;
    bool _alarm_active;
    bool _irq_flag;
    bc_scheduler_task_id_t _task_id;
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

//! @}

#endif // _BC_LIS2DH12_H
