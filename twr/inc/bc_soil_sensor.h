#ifndef _TWR_SOIL_SENSOR_H
#define _TWR_SOIL_SENSOR_H

#include <twr_ds28e17.h>
#include <twr_scheduler.h>
#include <twr_onewire.h>

//! @addtogroup twr_soil_sensor twr_soil_sensor
//! @brief Driver for soil sensor
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_SOIL_SENSOR_EVENT_ERROR = 0,

    //! @brief Update event
    TWR_SOIL_SENSOR_EVENT_UPDATE = 1,

} twr_soil_sensor_event_t;

//! @brief Error numbers

typedef enum
{
    //! @brief No error
    TWR_SOIL_SENSOR_ERROR_NONE = 0,

    //! @brief Sensor Module initialize error
    TWR_SOIL_SENSOR_ERROR_SENSOR_MODULE_INITIALIZE = 1,

    //! @brief Sensor Module onewire power up error
    TWR_SOIL_SENSOR_ERROR_SENSOR_MODULE_POWER_UP = 2,

    //! @brief No sensor found
    TWR_SOIL_SENSOR_ERROR_NO_SENSOR_FOUND = 3,

    //! @brief Tmp112 inicialize error
    TWR_SOIL_SENSOR_ERROR_TMP112_INITIALIZE = 4,

    //! @brief EEPROM header read error
    TWR_SOIL_SENSOR_ERROR_EEPROM_HEADER_READ = 5,

    //! @brief EEPROM signature error
    TWR_SOIL_SENSOR_ERROR_EEPROM_SIGNATURE= 6,

    //! @brief EEPROM version error
    TWR_SOIL_SENSOR_ERROR_EEPROM_VERSION = 7,

    //! @brief EEPROM payload length error
    TWR_SOIL_SENSOR_ERROR_EEPROM_PAYLOAD_LENGTH = 8,

    //! @brief EEPROM payload readerror
    TWR_SOIL_SENSOR_ERROR_EEPROM_PAYLOAD_READ = 9,

    //! @brief EEPROM payload crc error
    TWR_SOIL_SENSOR_ERROR_EEPROM_PAYLOAD_CRC = 10,

    //! @brief Zssc3123 inicialize error
    TWR_SOIL_SENSOR_ERROR_ZSSC3123_INITIALIZE = 11,

    //! @brief Tmp112 measurement request
    TWR_SOIL_SENSOR_ERROR_TMP112_MEASUREMENT_REQUEST = 12,

    //! @brief Zssc3123 measurement request
    TWR_SOIL_SENSOR_ERROR_ZSSC3123_MEASUREMENT_REQUEST = 13,

    //! @brief Tmp112 data fetch
    TWR_SOIL_SENSOR_ERROR_TMP112_DATA_FETCH = 14,

    //! @brief Zssc3123 data fetch
    TWR_SOIL_SENSOR_ERROR_ZSSC3123_DATA_FETCH = 15

} twr_soil_sensor_error_t;

//! @brief Soil sensor instance

typedef struct twr_soil_sensor_t twr_soil_sensor_t;

typedef struct twr_soil_sensor_sensor_t twr_soil_sensor_sensor_t;

#pragma pack(push, 1)

typedef struct
{
    //! @brief Signature 0xdeadbeef
    uint32_t signature;
    //! @brief Data Version
    uint8_t version;
    //! @brief Data Length
    uint8_t length;
    //! @brief Data CRC
    uint16_t crc;

} twr_soil_sensor_eeprom_header_t;

typedef struct
{
    //! @brief Product number
    uint8_t product;

    //! @brief Hardware Revision
    uint16_t revision;

    //! @brief Label
    char label[16 + 1];

    //! @brief Calibration points
    uint16_t calibration[11];

} twr_soil_sensor_eeprom_t;

#pragma pack(pop)

//! @cond

typedef enum
{
    TWR_SOIL_SENSOR_STATE_ERROR = -1,
    TWR_SOIL_SENSOR_STATE_PREINITIALIZE = 0,
    TWR_SOIL_SENSOR_STATE_INITIALIZE = 1,
    TWR_SOIL_SENSOR_STATE_READY = 2,
    TWR_SOIL_SENSOR_STATE_MEASURE = 3,
    TWR_SOIL_SENSOR_STATE_READ = 4,
    TWR_SOIL_SENSOR_STATE_UPDATE = 5

} twr_soil_sensor_state_t;

struct twr_soil_sensor_t
{
    twr_scheduler_task_id_t _task_id_interval;
    twr_scheduler_task_id_t _task_id_measure;
    twr_soil_sensor_state_t _state;
    bool _measurement_active;
    twr_tick_t _update_interval;
    void (*_event_handler)(twr_soil_sensor_t *, uint64_t, twr_soil_sensor_event_t, void *);
    void *_event_param;

    twr_onewire_t *_onewire;
    twr_soil_sensor_sensor_t *_sensor;
    int _sensor_count;
    int _sensor_found;
    twr_soil_sensor_error_t _error;
};

struct twr_soil_sensor_sensor_t
{
    twr_ds28e17_t _ds28e17;
    bool _temperature_valid;
    int16_t _temperature_raw;
    bool _cap_valid;
    uint16_t _cap_raw;
    twr_soil_sensor_eeprom_t _eeprom;
};

//! @endcond

//! @brief Initialize Soil sensor
//! @param[in] self Instance

void twr_soil_sensor_init(twr_soil_sensor_t *self);

//! @brief Initialize multiple Soil sensor
//! @param[in] self Instance
//! @param[in] sensors Sensors array
//! @param[in] sensor_count Sensors count

void twr_soil_sensor_init_multiple(twr_soil_sensor_t *self, twr_soil_sensor_sensor_t *sensors, int sensor_count);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_soil_sensor_set_event_handler(twr_soil_sensor_t *self, void (*event_handler)(twr_soil_sensor_t *, uint64_t, twr_soil_sensor_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void twr_soil_sensor_set_update_interval(twr_soil_sensor_t *self, twr_tick_t interval);

//! @brief Get sensors found
//! @param[in] self Instance
//! @return Number od found sensors

int twr_soil_sensor_get_sensor_found(twr_soil_sensor_t *self);

//! @brief Start measurement manually
//! @param[in] self Instance
//! @return true On success
//! @return false When other measurement is in progress

bool twr_soil_sensor_measure(twr_soil_sensor_t *self);

//! @brief Get measured temperature as raw value
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[out] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_soil_sensor_get_temperature_raw(twr_soil_sensor_t *self, uint64_t device_address, int16_t *raw);

//! @brief Get measured temperature in degrees of Celsius
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[in] celsius Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_soil_sensor_get_temperature_celsius(twr_soil_sensor_t *self, uint64_t device_address, float *celsius);

//! @brief Get measured temperature in degrees of Fahrenheit
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[in] fahrenheit Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_soil_sensor_get_temperature_fahrenheit(twr_soil_sensor_t *self, uint64_t device_address, float *fahrenheit);

//! @brief Get measured temperature in kelvin
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[in] kelvin Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_soil_sensor_get_temperature_kelvin(twr_soil_sensor_t *self, uint64_t device_address, float *kelvin);

//! @brief Get capacite as raw value
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[out] raw Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_soil_sensor_get_cap_raw(twr_soil_sensor_t *self, uint64_t device_address, uint16_t *raw);

//! @brief Get measured moisture in percent
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[in] moisture Pointer to variable where result will be stored
//! @return true When value is valid
//! @return false When value is invalid

bool twr_soil_sensor_get_moisture(twr_soil_sensor_t *self, uint64_t device_address, int *moisture);

//! @brief Get device index by its device address
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @return -1 Means no index for device address
//! @return > 0 Is valid index

int twr_soil_sensor_get_index_by_device_address(twr_soil_sensor_t *self, uint64_t device_address);

//! @brief Get device device address by its index
//! @param[in] self Instance
//! @param[in] index Index
//! @return 0 Means no device address for index
//! @return > 0 Is valid device address

uint64_t twr_soil_sensor_get_device_address_by_index(twr_soil_sensor_t *self, int index);

//! @brief Get device label by its device address
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @return Pointer to label

char *twr_soil_sensor_get_label(twr_soil_sensor_t *self, uint64_t device_address);

//! @brief Get device label by its device address
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[in] label New label
//! @return true On success
//! @return false When unknown device_address

bool twr_soil_sensor_set_label(twr_soil_sensor_t *self, uint64_t device_address, const char *label);

//! @brief Set value for calibration point by device address
//! @param[in] self Instance
//! @param[in] device_address 64b device address
//! @param[in] point 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100
//! @param[in] value
//! @return true On success
//! @return false When unknown device_address or bad point value

bool twr_soil_sensor_calibration_set_point(twr_soil_sensor_t *self, uint64_t device_address, uint8_t point, uint16_t value);

//! @brief Save calibration points and label to eeprom by device address
//! @param[in] self Instance
//! @param[in] device_address 64b device address

bool twr_soil_sensor_eeprom_save(twr_soil_sensor_t *self, uint64_t device_address);

//! @brief Get error number
//! @param[in] self Instance

twr_soil_sensor_error_t twr_soil_sensor_get_error(twr_soil_sensor_t *self);

//! @}

#endif // _TWR_SOIL_SENSOR_H
