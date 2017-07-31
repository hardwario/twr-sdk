#ifndef _BC_DS18B20_H
#define _BC_DS18B20_H

#include <bc_1wire.h>
#include <bc_module_sensor.h>
#include <bc_scheduler.h>

//! @addtogroup bc_ds18b20 bc_ds18b20
//! @brief Driver for 1-wire dalas temperature, supports the following devices : DS18B20
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
	BC_DS18B20_EVENT_ERROR = 0,

    //! @brief Update event
	BC_DS18B20_EVENT_UPDATE = 1

} bc_ds18b20_event_t;


typedef enum
{
    //! @brief
	BC_DS18B20_BIT_RESOLUTION_9 = 0,

    //! @brief
	BC_DS18B20_BIT_RESOLUTION_10 = 1,

	BC_DS18B20_BIT_RESOLUTION_11 = 2,

	BC_DS18B20_BIT_RESOLUTION_12 = 3

} bc_ds18b20_bit_resolution_t;


//! @brief 1-wire dalas temperature instance

typedef struct bc_ds18b20_t bc_ds18b20_t;

//! @cond

typedef enum
{
	BC_DS18B20_STATE_ERROR = -1,
	BC_DS18B20_STATE_INITIALIZE = 0,
	BC_DS18B20_STATE_READ_SCRATCHPAD = 1,
	BC_DS18B20_STATE_MEASURE = 2,
	BC_DS18B20_STATE_READ = 3,
	BC_DS18B20_STATE_UPDATE = 4

} bc_ds18b20_state_t;

struct bc_ds18b20_t
{
	uint64_t _device_number;
	bc_module_sensor_channel_t _channel;
	bc_gpio_channel_t _gpio_channel;
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(bc_ds18b20_t *, bc_ds18b20_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    bc_ds18b20_state_t _state;
    bc_tick_t _tick_ready;
    bc_tick_t _update_interval;
    bool _temperature_valid;
    uint8_t _scratchpad[9];
};

//! @endcond

//! @brief Initialize 1-wire dalas temperature
//! @param[in] self Instance
//! @param[in] device_number
//! @param[in] channel
//! @return true On success
//! @return false On failure

bool bc_ds18b20_init(bc_ds18b20_t *self, uint64_t device_number, bc_module_sensor_channel_t channel);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_ds18b20_set_event_handler(bc_ds18b20_t *self, void (*event_handler)(bc_ds18b20_t *, bc_ds18b20_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_ds18b20_set_update_interval(bc_ds18b20_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_ds18b20_measure(bc_ds18b20_t *self);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[in] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalids

bool bc_ds18b20_get_temperature_raw(bc_ds18b20_t *self, int16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_ds18b20_get_temperature_celsius(bc_ds18b20_t *self, float *celsius);

//! @brief Get resolution
//! @param[in] self Instance
//! @param[in] resolution Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_ds18b20_get_resolution(bc_ds18b20_t *self, bc_ds18b20_bit_resolution_t *resolution);

//! @brief Get conversion time
//! @param[in] self Instance
//! @param[in] conversion_time Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_ds18b20_get_conversion_time(bc_ds18b20_t *self, bc_tick_t *conversion_time);


//! @}

#endif // _BC_DS18B20_H
