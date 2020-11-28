#ifndef _HIO_DS18B20_H
#define _HIO_DS18B20_H

#include <hio_tick.h>
#include <hio_module_sensor.h>
#include <hio_scheduler.h>
#include <hio_onewire.h>

//! @addtogroup hio_ds18b20 hio_ds18b20
//! @brief Driver for 1wire ds18b20
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_DS18B20_EVENT_ERROR = -1,

    //! @brief Update event
    HIO_DS18B20_EVENT_UPDATE = 0,

} hio_ds18b20_event_t;

//! @brief BigClown ds18b20 instance

typedef struct hio_ds18b20_t hio_ds18b20_t;
typedef struct hio_ds18b20_sensor_t hio_ds18b20_sensor_t;

//! @cond

typedef enum
{
    HIO_DS18B20_STATE_ERROR = -1,
    HIO_DS18B20_STATE_PREINITIALIZE = 0,
    HIO_DS18B20_STATE_INITIALIZE = 1,
    HIO_DS18B20_STATE_READY = 2,
    HIO_DS18B20_STATE_MEASURE = 3,
    HIO_DS18B20_STATE_READ = 4,
    HIO_DS18B20_STATE_UPDATE = 5

} hio_ds18b20_state_t;

typedef enum
{
    HIO_DS18B20_RESOLUTION_BITS_9 = 0,
    HIO_DS18B20_RESOLUTION_BITS_10 = 1,
    HIO_DS18B20_RESOLUTION_BITS_11 = 2,
    HIO_DS18B20_RESOLUTION_BITS_12 = 3

} hio_ds18b20_resolution_bits_t;

struct hio_ds18b20_sensor_t
{
    int16_t _temperature_raw;
    uint64_t _device_address;
    bool _temperature_valid;
};

struct hio_ds18b20_t
{
    hio_scheduler_task_id_t _task_id_interval;
    hio_scheduler_task_id_t _task_id_measure;
    void (*_event_handler)(hio_ds18b20_t *, uint64_t _device_address, hio_ds18b20_event_t, void *);
    void *_event_param;
    bool _measurement_active;
    hio_tick_t _update_interval;
    hio_ds18b20_state_t _state;

    hio_ds18b20_sensor_t *_sensor;
    int _sensor_count;
    int _sensor_found;

    hio_ds18b20_resolution_bits_t _resolution;

    bool _power;
    bool _power_dynamic;
    hio_onewire_t *_onewire;
};

//! @endcond


//! @brief Initialize single ds18b20 over channel B on Sensor Module
//! @param[in] self Instance
//! @param[in] resolution Resolution

void hio_ds18b20_init_single(hio_ds18b20_t *self, hio_ds18b20_resolution_bits_t resolution);

//! @brief Initialize multiple ds18b20 over channel B on Sensor Module
//! @param[in] self Instance
//! @param[in] sensors Sensor array
//! @param[in] sensor_count Max count sensors
//! @param[in] resolution Resolution

void hio_ds18b20_init_multiple(hio_ds18b20_t *self, hio_ds18b20_sensor_t *sensors, int sensor_count, hio_ds18b20_resolution_bits_t resolution);

//! @brief Initialize ds18b20
//! @param[in] self Instance
//! @param[in] onewire Pointer on instance 1-Wire
//! @param[in] sensors Sensor array
//! @param[in] sensor_count Max count sensors
//! @param[in] resolution Resolution

void hio_ds18b20_init(hio_ds18b20_t *self, hio_onewire_t *onewire, hio_ds18b20_sensor_t *sensors, int sensor_count, hio_ds18b20_resolution_bits_t resolution);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_ds18b20_set_event_handler(hio_ds18b20_t *self, void (*event_handler)(hio_ds18b20_t *, uint64_t _device_address, hio_ds18b20_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void hio_ds18b20_set_update_interval(hio_ds18b20_t *self, hio_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool hio_ds18b20_measure(hio_ds18b20_t *self);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @brief Get measured temperature as raw values
//! @return true When value is valid
//! @return false When value is invalid

bool hio_ds18b20_get_temperature_raw(hio_ds18b20_t *self, uint64_t device_address, int16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[in] celsius Pointer to variables where results will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool hio_ds18b20_get_temperature_celsius(hio_ds18b20_t *self, uint64_t _device_address, float *celsius);

//! @brief Get device index by its device address
//! @param[in] self Instance
//! @param[in] device_address 64b device address

int hio_ds18b20_get_index_by_device_address(hio_ds18b20_t *self, uint64_t device_address);

//! @brief Get device index by its device address
//! @param[in] self Instance
//! @param[in] index Index
//! @return short addres or 0

uint64_t hio_ds182b0_get_short_address(hio_ds18b20_t *self, uint8_t index);

//! @brief Get number of found sensor
//! @param[in] self Instance

int hio_ds18b20_get_sensor_found(hio_ds18b20_t *self);

//! @brief Set power dynamic, Turns VDD on and pull 4k7 only for measurement
//! @param[in] self Instance
//! @param[in] on True enable dynamic, False disable dynamic

void hio_ds18b20_set_power_dynamic(hio_ds18b20_t *self, bool on);

//! @}

#endif // _HIO_DS18B20_H
