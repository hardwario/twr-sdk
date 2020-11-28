#ifndef _BC_ZSSC3123_H
#define _BC_ZSSC3123_H

#include <bc_i2c.h>
#include <bc_scheduler.h>

//! @addtogroup bc_zssc3123 bc_zssc3123
//! @brief Driver for ZSSC3123 Capacitive Sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_ZSSC3123_EVENT_ERROR = 0,

    //! @brief Update event
    BC_ZSSC3123_EVENT_UPDATE = 1

} bc_zssc3123_event_t;

//! @brief ZSSC3123 instance

typedef struct bc_zssc3123_t bc_zssc3123_t;

//! @cond

typedef enum
{
    BC_ZSSC3123_STATE_ERROR = -1,
    BC_ZSSC3123_STATE_INITIALIZE = 0,
    BC_ZSSC3123_STATE_MEASURE = 1,
    BC_ZSSC3123_STATE_READ = 2

} bc_zssc3123_state_t;

struct bc_zssc3123_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    bc_scheduler_task_id_t _task_id;
    void (*_event_handler)(bc_zssc3123_t *, bc_zssc3123_event_t, void *);
    void *_event_param;
    bc_zssc3123_state_t _state;
    bc_tick_t _update_interval;
    bool _valid;
    bool _measurement_active;
    bc_tick_t _next_update_start;
    uint16_t _raw;
    bc_tick_t _data_fetch_delay;
};

//! @endcond

//! @brief Initialize ZSSC3123
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address

bool bc_zssc3123_init(bc_zssc3123_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address);

//! @brief Deitialize ZSSC3123
//! @param[in] self Instance

bool bc_zssc3123_deinit(bc_zssc3123_t *self);

//! @brief Set data fetch delay
//! @param[in] self Instance
//! @param[in] interval Desired scan interval in ticks

void bc_zssc3123_set_data_fetch_delay(bc_zssc3123_t *self, bc_tick_t interval);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_zssc3123_set_event_handler(bc_zssc3123_t *self, void (*event_handler)(bc_zssc3123_t *, bc_zssc3123_event_t, void *), void *event_param);

//! @brief Set scan interval
//! @param[in] self Instance
//! @param[in] interval Desired scan interval in ticks

void bc_zssc3123_set_update_interval(bc_zssc3123_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_zssc3123_measure(bc_zssc3123_t *self);

//! @brief Get capacitance data as raw value
//! @param[in] self Instance
//! @param[out] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_zssc3123_get_raw_cap_data(bc_zssc3123_t *self, uint16_t *raw);

//! @brief Start Command Mode
//! @param[in] self Instance

bool bc_zssc3123_start_cm(bc_zssc3123_t *self);

//! @brief Ends Command Mode
//! @param[in] self Instance

bool bc_zssc3123_end_cm(bc_zssc3123_t *self);

//! @brief Write to eeprom, work only with command mode
//! @param[in] self Instance
//! @param[in] address Address
//! @param[out] word Output data which have been read
//! @return true On success
//! @return false On failure

bool bc_zssc3123_eeprom_read(bc_zssc3123_t *self, uint8_t address, uint16_t *word);

//! @brief Read from eeprm, work only with command mode
//! @param[in] self Instance
//! @param[in] address Address
//! @param[in] word Data to write
//! @return true On success
//! @return false On failure

bool bc_zssc3123_eeprom_write(bc_zssc3123_t *self, uint8_t address, uint16_t word);

//! @brief Unlock eerpom, work only with command mode
//! @param[in] self Instance
//! @return true On success
//! @return false On failure

bool bc_zssc3123_unlock_eeprom(bc_zssc3123_t *self);

//! @}

#endif // _BC_ZSSC3123_H
