#ifndef _HIO_CY8CMBR3102_H
#define _HIO_CY8CMBR3102_H

#include <hio_i2c.h>
#include <hio_scheduler.h>

//! @addtogroup hio_cy8cmbr3102 hio_cy8cmbr3102
//! @brief Driver for CY8CMBR3102
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_CY8CMBR3102_EVENT_ERROR = 0,

    //! @brief Update event
    HIO_CY8CMBR3102_EVENT_TOUCH = 1

} hio_cy8cmbr3102_event_t;

//! @brief TCA9534A instance

typedef struct hio_cy8cmbr3102_t hio_cy8cmbr3102_t;

//! @cond

typedef enum
{
    HIO_CY8CMBR3102_STATE_ERROR = -1,
    HIO_CY8CMBR3102_STATE_INITIALIZE = 0,
    HIO_CY8CMBR3102_STATE_CALC_CONFIG_CRC = 1,
    HIO_CY8CMBR3102_STATE_SELF_RESET = 2,
    HIO_CY8CMBR3102_STATE_READ = 3

} hio_cy8cmbr3102_state_t;

struct hio_cy8cmbr3102_t
{
    hio_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    hio_scheduler_task_id_t _task_id_task;
    void (*_event_handler)(hio_cy8cmbr3102_t *, hio_cy8cmbr3102_event_t, void *);
    void *_event_param;
    hio_cy8cmbr3102_state_t _state;
    hio_tick_t _scan_interval;
    int _error_cnt;
};

//! @endcond

//! @brief Initialize CY8CMBR3102
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

bool hio_cy8cmbr3102_init(hio_cy8cmbr3102_t *self, hio_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_cy8cmbr3102_set_event_handler(hio_cy8cmbr3102_t *self, void (*event_handler)(hio_cy8cmbr3102_t *, hio_cy8cmbr3102_event_t, void *), void *event_param);

//! @brief Set scan interval
//! @param[in] self Instance
//! @param[in] scan_interval Desired scan interval in ticks

void hio_cy8cmbr3102_set_scan_interval(hio_cy8cmbr3102_t *self, hio_tick_t scan_interval);

//! @brief Get proximity (Capacitive sensor difference count signal.)
//! @param[in] self Instance
//! @param[out] value
//! @return true On success
//! @return false On failure

bool hio_cy8cmbr3102_get_proximity(hio_cy8cmbr3102_t *self, uint16_t value);

//! @brief Is touch
//! @param[in] self Instance
//! @param[out] is_touch
//! @return true On success
//! @return false On failure

bool hio_cy8cmbr3102_is_touch(hio_cy8cmbr3102_t *self, bool *is_touch);

//! @}

#endif // _HIO_CY8CMBR3102_H
