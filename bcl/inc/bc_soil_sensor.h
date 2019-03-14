#ifndef _BC_SOIL_SENSOR_H
#define _BC_SOIL_SENSOR_H

#include <bc_ds28e17.h>
#include <bc_scheduler.h>

//! @addtogroup bc_soil_sensor bc_soil_sensor
//! @brief Driver for soil sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_SOIL_SENSOR_EVENT_ERROR = 0,

    //! @brief Update event
    BC_SOIL_SENSOR_EVENT_UPDATE = 1,

} bc_soil_sensor_event_t;

//! @brief Soil sensor instance

typedef struct bc_soil_sensor_t bc_soil_sensor_t;

typedef struct bc_soil_sensor_sensor_t bc_soil_sensor_sensor_t;

#pragma pack(push, 1)

typedef struct
{
    //! @brief Signature 0x4b425048
    uint32_t signature;
    uint8_t version;
    uint8_t length;
    uint16_t crc;

} bc_soil_sensor_eeprom_header_t;

typedef struct
{
    //! @brief Sensor revision
    uint16_t revision;

    //! @brief Calibration points
    uint16_t calibration[11];

    //! @brief Label
    char label[16 + 1];

} bc_soil_sensor_eeprom_t;

#pragma pack(pop)

//! @cond

typedef enum
{
    BC_SOIL_SENSOR_STATE_ERROR = -1,
    BC_SOIL_SENSOR_STATE_PREINITIALIZE = 0,
    BC_SOIL_SENSOR_STATE_INITIALIZE = 1,
    BC_SOIL_SENSOR_STATE_READY = 2,
    BC_SOIL_SENSOR_STATE_MEASURE = 3,
    BC_SOIL_SENSOR_STATE_READ = 4,
    BC_SOIL_SENSOR_STATE_UPDATE = 5

} bc_soil_sensor_state_t;

struct bc_soil_sensor_t
{
    bc_scheduler_task_id_t _task_id_interval;
    bc_scheduler_task_id_t _task_id_measure;
    bc_soil_sensor_state_t _state;
    bool _measurement_active;
    bc_tick_t _update_interval;
    void (*_event_handler)(bc_soil_sensor_t *, uint64_t device_address, bc_soil_sensor_event_t, void *);
    void *_event_param;

    bc_gpio_channel_t _channel;
    bc_soil_sensor_sensor_t *_sensor;
    int _sensor_count;
    int _sensor_found;
    bc_ds28e17_t _ds28e17;
};

struct bc_soil_sensor_sensor_t
{
    bc_ds28e17_t _ds28e17;
    bool _temperature_valid;
    int16_t _temperature_raw;
    bool _cap_valid;
    uint16_t _cap_raw;
    bc_soil_sensor_eeprom_t eeprom;
};

//! @endcond

//! @brief Initialize Soil sensor
//! @param[in] self Instance
//! @param[in] device_number Device number

void bc_soil_sensor_init(bc_soil_sensor_t *self);

void bc_soil_sensor_init_multiple(bc_soil_sensor_t *self, bc_soil_sensor_sensor_t *sensors, int sensor_count);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_soil_sensor_set_event_handler(bc_soil_sensor_t *self, void (*event_handler)(bc_soil_sensor_t *, uint64_t device_address, bc_soil_sensor_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_soil_sensor_set_update_interval(bc_soil_sensor_t *self, bc_tick_t interval);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool bc_soil_sensor_measure(bc_soil_sensor_t *self);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[out] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_soil_sensor_get_temperature_raw(bc_soil_sensor_t *self, uint64_t device_address, int16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_soil_sensor_get_temperature_celsius(bc_soil_sensor_t *self, uint64_t device_address, float *celsius);

//! @brief Get measured temperature in degrees of Fahrenheit
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[in] fahrenheit Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_soil_sensor_get_temperature_fahrenheit(bc_soil_sensor_t *self, uint64_t device_address, float *fahrenheit);

//! @brief Get measured temperature in kelvin
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[in] kelvin Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_soil_sensor_get_temperature_kelvin(bc_soil_sensor_t *self, uint64_t device_address, float *kelvin);

//! @brief Get capacite as raw value
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[out] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_soil_sensor_get_cap_raw(bc_soil_sensor_t *self, uint64_t device_address, uint16_t *raw);

//! @brief Get measured moisture in percent
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[in] moisture Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool bc_soil_sensor_get_moisture(bc_soil_sensor_t *self, uint64_t device_address, int *moisture);

//! @brief Get device index by its device address
//! @param[in] self Instance
//! @param[in] device_address 64b device address

int bc_soil_sensor_get_index_by_device_address(bc_soil_sensor_t *self, uint64_t device_address);

//! @brief Get device label by its device address
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @return Pointer to label

char *bc_soil_sensor_get_label(bc_soil_sensor_t *self, uint64_t device_address);

//! @brief Get device label by its device address
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[in] label New label
//! @return true On success
//! @return false When unknown device_address

bool bc_soil_sensor_set_label(bc_soil_sensor_t *self, uint64_t device_address, const char *label);

//! @brief Set value for calibration point by device address
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[in] point 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100
//! @param[in] value
//! @return true On success
//! @return false When unknown device_address or bad point value

bool bc_soil_sensor_calibration_set_point(bc_soil_sensor_t *self, uint64_t device_address, uint8_t point, uint16_t value);

bool bc_soil_sensor_eeprom_save(bc_soil_sensor_t *self, uint64_t device_address);

//! @}

#endif // _BC_SOIL_SENSOR_H
