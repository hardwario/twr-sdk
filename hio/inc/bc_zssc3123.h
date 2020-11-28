#ifndef _HIO_ZSSC3123_H
#define _HIO_ZSSC3123_H

#include <hio_i2c.h>
#include <hio_scheduler.h>

//! @addtogroup hio_zssc3123 hio_zssc3123
//! @brief Driver for ZSSC3123 Capacitive Sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_ZSSC3123_EVENT_ERROR = 0,

    //! @brief Update event
    HIO_ZSSC3123_EVENT_UPDATE = 1

} hio_zssc3123_event_t;

//! @brief ZSSC3123 instance

typedef struct hio_zssc3123_t hio_zssc3123_t;

//! @cond

typedef enum
{
    HIO_ZSSC3123_STATE_ERROR = -1,
    HIO_ZSSC3123_STATE_INITIALIZE = 0,
    HIO_ZSSC3123_STATE_MEASURE = 1,
    HIO_ZSSC3123_STATE_READ = 2

} hio_zssc3123_state_t;

struct hio_zssc3123_t
{
    hio_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    hio_scheduler_task_id_t _task_id;
    void (*_event_handler)(hio_zssc3123_t *, hio_zssc3123_event_t, void *);
    void *_event_param;
    hio_zssc3123_state_t _state;
    hio_tick_t _update_interval;
    bool _valid;
    bool _measurement_active;
    hio_tick_t _next_update_start;
    uint16_t _raw;
    hio_tick_t _data_fetch_delay;
};

//! @endcond

//! @brief Initialize ZSSC3123
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

bool hio_zssc3123_init(hio_zssc3123_t *self, hio_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Deitialize ZSSC3123
//! @param[in] self Instance

bool hio_zssc3123_deinit(hio_zssc3123_t *self);

//! @brief Set data fetch delay
//! @param[in] self Instance
//! @param[in] interval Desired scan interval in ticks

void hio_zssc3123_set_data_fetch_delay(hio_zssc3123_t *self, hio_tick_t interval);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_zssc3123_set_event_handler(hio_zssc3123_t *self, void (*event_handler)(hio_zssc3123_t *, hio_zssc3123_event_t, void *), void *event_param);

//! @brief Set scan interval
//! @param[in] self Instance
//! @param[in] interval Desired scan interval in ticks

void hio_zssc3123_set_update_interval(hio_zssc3123_t *self, hio_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool hio_zssc3123_measure(hio_zssc3123_t *self);

//! @brief Get capacitance data as raw value
//! @param[in] self Instance
//! @param[out] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_zssc3123_get_raw_cap_data(hio_zssc3123_t *self, uint16_t *raw);

//! @brief Start Command Mode
//! @param[in] self Instance

bool hio_zssc3123_start_cm(hio_zssc3123_t *self);

//! @brief Ends Command Mode
//! @param[in] self Instance

bool hio_zssc3123_end_cm(hio_zssc3123_t *self);

//! @brief Write to eeprom, work only with command mode
//! @param[in] self Instance
//! @param[in] address Address
//! @param[out] word Output data which have been read
//! @return true On success
//! @return false On failure

bool hio_zssc3123_eeprom_read(hio_zssc3123_t *self, uint8_t address, uint16_t *word);

//! @brief Read from eeprm, work only with command mode
//! @param[in] self Instance
//! @param[in] address Address
//! @param[in] word Data to write
//! @return true On success
//! @return false On failure

bool hio_zssc3123_eeprom_write(hio_zssc3123_t *self, uint8_t address, uint16_t word);

//! @brief Unlock eerpom, work only with command mode
//! @param[in] self Instance
//! @return true On success
//! @return false On failure

bool hio_zssc3123_unlock_eeprom(hio_zssc3123_t *self);

//! @}

#endif // _HIO_ZSSC3123_H
