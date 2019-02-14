#ifndef _BC_APDS9960_H
#define _BC_APDS9960_H

#include <bc_common.h>
#include <bc_i2c.h>
#include <bc_scheduler.h>

#define BC_APDS9960_BUFFER_FIFO_LEVEL 32

//! @addtogroup bc_apds9960 bc_apds9960
//! @brief Driver for APDS-9960 RGB and Gesture Sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_APDS9960_EVENT_ERROR = 0,

    //! @brief Update event
    BC_APDS9960_EVENT_UPDATE = 1

} bc_apds9960_event_t;

//! @brief LED drive current

typedef enum
{
    //! @brief 100 mA
    BC_APDS9960_LDRIVE_100MA = 0 << 6,

    //! @brief 50 mA
    BC_APDS9960_LDRIVE_50MA = 1 << 6,

    //! @brief 25 mA
    BC_APDS9960_LDRIVE_25MA = 2 << 6,

    //! @brief 12.5 mA
    BC_APDS9960_LDRIVE_12MA = 3 << 6,

} bc_apds9960_ldrive_t;

//! @brief Proximity Gain Control

typedef enum
{
    //! @brief 1x
    BC_APDS9960_PGAIN_1X = 0 << 2,

    //! @brief 2x
    BC_APDS9960_PGAIN_2X = 1 << 2,

    //! @brief 4x
    BC_APDS9960_PGAIN_4X = 2 << 2,

    //! @brief 8x
    BC_APDS9960_PGAIN_8X = 3 << 2,

} bc_apds9960_pgain_t;

//! @brief ALS and Color Gain Control

typedef enum
{
    //! @brief 1x
    BC_APDS9960_AGAIN_1X = 0,

    //! @brief 4x
    BC_APDS9960_AGAIN_4X = 1,

    //! @brief 16x
    BC_APDS9960_AGAIN_16X = 2,

    //! @brief 64x
    BC_APDS9960_AGAIN_64X = 3,

} bc_apds9960_again_t;


typedef enum {
    BC_APDS9960_GESTURE_NONE,
    BC_APDS9960_GESTURE_LEFT,
    BC_APDS9960_GESTURE_RIGHT,
    BC_APDS9960_GESTURE_UP,
    BC_APDS9960_GESTURE_DOWN,
    BC_APDS9960_GESTURE_NEAR,
    BC_APDS9960_GESTURE_FAR,
    BC_APDS9960_GESTURE_ALL

} bc_apds9960_gesture_t;


//! @brief APDS9960 instance

typedef struct bc_apds9960_t bc_apds9960_t;

//! @cond

typedef enum
{
    BC_APDS9960_STATE_ERROR = -1,
    BC_APDS9960_STATE_INITIALIZE = 0,
    BC_APDS9960_STATE_READY = 1,
    BC_APDS9960_STATE_GESTURE_READ_DATA = 2,


    BC_APDS9960_STATE_GESTURE_READ_PROXIMITY = 3

} bc_apds9960_state_t;

struct bc_apds9960_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    void (*_event_handler)(bc_apds9960_t *, bc_apds9960_event_t, void *);
    void *_event_param;
    bc_scheduler_task_id_t _task_id;
    bc_tick_t _update_interval;
    bc_apds9960_state_t _state;
    uint8_t _control_reg;
    bool _irq_flag;

    bc_apds9960_gesture_t _gesture;

    uint8_t _buffer[BC_APDS9960_BUFFER_FIFO_LEVEL * 4];
    uint8_t _buffer_length;

    int gesture_ud_delta_;
    int gesture_lr_delta_;
    int gesture_ud_count_;
    int gesture_lr_count_;
    int gesture_near_count_;
    int gesture_far_count_;
    int gesture_state_;

    uint8_t _p;

};

//! @endcond

//! @brief Initialize APDS9960
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

void bc_apds9960_init(bc_apds9960_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_apds9960_set_event_handler(bc_apds9960_t *self, void (*event_handler)(bc_apds9960_t *, bc_apds9960_event_t, void *), void *event_param);

//! @brief Set LED drive current
//! @param[in] self Instance
//! @param[in] low
//! @param[in] high

void bc_apds9960_set_proximity_threshold(bc_apds9960_t *self, uint8_t low, uint8_t high);

bool bc_apds9960_get_proximity(bc_apds9960_t *self, uint8_t *value);

void bc_apds9960_get_gesture(bc_apds9960_t *self, bc_apds9960_gesture_t *gesture);

//! @}

#endif // _BC_APDS9960_H
