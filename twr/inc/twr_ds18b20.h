#ifndef _TWR_DS18B20_H
#define _TWR_DS18B20_H

#include <twr_tick.h>
#include <twr_module_sensor.h>
#include <twr_scheduler.h>
#include <twr_onewire.h>

//! @addtogroup twr_ds18b20 twr_ds18b20
//! @brief Driver for 1wire ds18b20
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_DS18B20_EVENT_ERROR = -1,

    //! @brief Update event
    TWR_DS18B20_EVENT_UPDATE = 0,

} twr_ds18b20_event_t;

//! @brief BigClown ds18b20 instance

typedef struct twr_ds18b20_t twr_ds18b20_t;
typedef struct twr_ds18b20_sensor_t twr_ds18b20_sensor_t;

//! @cond

typedef enum
{
    TWR_DS18B20_STATE_ERROR = -1,
    TWR_DS18B20_STATE_PREINITIALIZE = 0,
    TWR_DS18B20_STATE_INITIALIZE = 1,
    TWR_DS18B20_STATE_READY = 2,
    TWR_DS18B20_STATE_MEASURE = 3,
    TWR_DS18B20_STATE_READ = 4,
    TWR_DS18B20_STATE_RESULTS = 5

} twr_ds18b20_state_t;

typedef enum
{
    TWR_DS18B20_RESOLUTION_BITS_9 = 0,
    TWR_DS18B20_RESOLUTION_BITS_10 = 1,
    TWR_DS18B20_RESOLUTION_BITS_11 = 2,
    TWR_DS18B20_RESOLUTION_BITS_12 = 3

} twr_ds18b20_resolution_bits_t;

struct twr_ds18b20_sensor_t
{
    int16_t _temperature_raw;
    uint64_t _device_address;
    bool _temperature_valid;
};

struct twr_ds18b20_t
{
    twr_scheduler_task_id_t _task_id_interval;
    twr_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(twr_ds18b20_t *, uint64_t _device_address, twr_ds18b20_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    twr_tick_t _update_interval;
    twr_ds18b20_state_t _state;

    twr_ds18b20_sensor_t *_sensor;
    int _sensor_count;
    int _sensor_found;

    twr_ds18b20_resolution_bits_t _resolution;

    bool _power;
    bool _power_dynamic;
    twr_onewire_t *_onewire;
};

//! @endcond


//! @brief Initialize single ds18b20 over channel B on Sensor Module
//! @param[in] self Instance
//! @param[in] resolution Resolution

void twr_ds18b20_init_single(twr_ds18b20_t *self, twr_ds18b20_resolution_bits_t resolution);

//! @brief Initialize multiple ds18b20 over channel B on Sensor Module
//! @param[in] self Instance
//! @param[in] sensors Sensor array
//! @param[in] sensor_count Max count sensors
//! @param[in] resolution Resolution

void twr_ds18b20_init_multiple(twr_ds18b20_t *self, twr_ds18b20_sensor_t *sensors, int sensor_count, twr_ds18b20_resolution_bits_t resolution);

//! @brief Initialize ds18b20
//! @param[in] self Instance
//! @param[in] onewire Pointer on instance 1-Wire
//! @param[in] sensors Sensor array
//! @param[in] sensor_count Max count sensors
//! @param[in] resolution Resolution

void twr_ds18b20_init(twr_ds18b20_t *self, twr_onewire_t *onewire, twr_ds18b20_sensor_t *sensors, int sensor_count, twr_ds18b20_resolution_bits_t resolution);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_ds18b20_set_event_handler(twr_ds18b20_t *self, void (*event_handler)(twr_ds18b20_t *, uint64_t _device_address, twr_ds18b20_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_ds18b20_set_update_interval(twr_ds18b20_t *self, twr_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_ds18b20_measure(twr_ds18b20_t *self);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @brief Get measured temperature as raw values
//! @return true When value is valid
//! @return false When value is invalid

bool twr_ds18b20_get_temperature_raw(twr_ds18b20_t *self, uint64_t device_address, int16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[in] celsius Pointer to variables where results will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_ds18b20_get_temperature_celsius(twr_ds18b20_t *self, uint64_t _device_address, float *celsius);

//! @brief Get device index by its device address
//! @param[in] self Instance
//! @param[in] device_address 64b device address

int twr_ds18b20_get_index_by_device_address(twr_ds18b20_t *self, uint64_t device_address);

//! @brief Get device index by its device address
//! @param[in] self Instance
//! @param[in] index Index
//! @return short addres or 0

uint64_t twr_ds182b0_get_short_address(twr_ds18b20_t *self, uint8_t index);

//! @brief Get number of found sensor
//! @param[in] self Instance

int twr_ds18b20_get_sensor_found(twr_ds18b20_t *self);

//! @brief Set power dynamic, Turns VDD on and pull 4k7 only for measurement
//! @param[in] self Instance
//! @param[in] on True enable dynamic, False disable dynamic

void twr_ds18b20_set_power_dynamic(twr_ds18b20_t *self, bool on);

//! @}

#endif // _TWR_DS18B20_H
