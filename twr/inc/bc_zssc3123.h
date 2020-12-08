#ifndef _TWR_ZSSC3123_H
#define _TWR_ZSSC3123_H

#include <twr_i2c.h>
#include <twr_scheduler.h>

//! @addtogroup twr_zssc3123 twr_zssc3123
//! @brief Driver for ZSSC3123 Capacitive Sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_ZSSC3123_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_ZSSC3123_EVENT_UPDATE = 1

} twr_zssc3123_event_t;

//! @brief ZSSC3123 instance

typedef struct twr_zssc3123_t twr_zssc3123_t;

//! @cond

typedef enum
{
    TWR_ZSSC3123_STATE_ERROR = -1,
    TWR_ZSSC3123_STATE_INITIALIZE = 0,
    TWR_ZSSC3123_STATE_MEASURE = 1,
    TWR_ZSSC3123_STATE_READ = 2

} twr_zssc3123_state_t;

struct twr_zssc3123_t
{
    twr_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    twr_scheduler_task_id_t _task_id;
    void (*_event_handler)(twr_zssc3123_t *, twr_zssc3123_event_t, void *);
    void *_event_param;
    twr_zssc3123_state_t _state;
    twr_tick_t _update_interval;
    bool _valid;
    bool _measurement_active;
    twr_tick_t _next_update_start;
    uint16_t _raw;
    twr_tick_t _data_fetch_delay;
};

//! @endcond

//! @brief Initialize ZSSC3123
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

bool twr_zssc3123_init(twr_zssc3123_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Deitialize ZSSC3123
//! @param[in] self Instance

bool twr_zssc3123_deinit(twr_zssc3123_t *self);

//! @brief Set data fetch delay
//! @param[in] self Instance
//! @param[in] interval Desired scan interval in ticks

void twr_zssc3123_set_data_fetch_delay(twr_zssc3123_t *self, twr_tick_t interval);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_zssc3123_set_event_handler(twr_zssc3123_t *self, void (*event_handler)(twr_zssc3123_t *, twr_zssc3123_event_t, void *), void *event_param);

//! @brief Set scan interval
//! @param[in] self Instance
//! @param[in] interval Desired scan interval in ticks

void twr_zssc3123_set_update_interval(twr_zssc3123_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_zssc3123_measure(twr_zssc3123_t *self);

//! @brief Get capacitance data as raw value
//! @param[in] self Instance
//! @param[out] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_zssc3123_get_raw_cap_data(twr_zssc3123_t *self, uint16_t *raw);

//! @brief Start Command Mode
//! @param[in] self Instance

bool twr_zssc3123_start_cm(twr_zssc3123_t *self);

//! @brief Ends Command Mode
//! @param[in] self Instance

bool twr_zssc3123_end_cm(twr_zssc3123_t *self);

//! @brief Write to eeprom, work only with command mode
//! @param[in] self Instance
//! @param[in] address Address
//! @param[out] word Output data which have been read
//! @return true On success
//! @return false On failure

bool twr_zssc3123_eeprom_read(twr_zssc3123_t *self, uint8_t address, uint16_t *word);

//! @brief Read from eeprm, work only with command mode
//! @param[in] self Instance
//! @param[in] address Address
//! @param[in] word Data to write
//! @return true On success
//! @return false On failure

bool twr_zssc3123_eeprom_write(twr_zssc3123_t *self, uint8_t address, uint16_t word);

//! @brief Unlock eerpom, work only with command mode
//! @param[in] self Instance
//! @return true On success
//! @return false On failure

bool twr_zssc3123_unlock_eeprom(twr_zssc3123_t *self);

//! @}

#endif // _TWR_ZSSC3123_H
