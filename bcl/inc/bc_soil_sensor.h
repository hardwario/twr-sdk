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

//! @brief Error numbers

typedef enum
{
    //! @brief No error
    BC_SOIL_SENSOR_ERROR_NONE = 0,

    //! @brief Sensor Module initialize error
    BC_SOIL_SENSOR_ERROR_SENSOR_MODULE_INITIALIZE = 1,

    //! @brief Sensor Module onewire power up error
    BC_SOIL_SENSOR_ERROR_SENSOR_MODULE_POWER_UP = 2,

    //! @brief No sensor found
    BC_SOIL_SENSOR_ERROR_NO_SENSOR_FOUND = 3,

    //! @brief Tmp112 inicialize error
    BC_SOIL_SENSOR_ERROR_TMP112_INITIALIZE = 4,

    //! @brief EEPROM header read error
    BC_SOIL_SENSOR_ERROR_EEPROM_HEADER_READ = 5,

    //! @brief EEPROM signature error
    BC_SOIL_SENSOR_ERROR_EEPROM_SIGNATURE= 6,

    //! @brief EEPROM version error
    BC_SOIL_SENSOR_ERROR_EEPROM_VERSION = 7,

    //! @brief EEPROM payload length error
    BC_SOIL_SENSOR_ERROR_EEPROM_PAYLOAD_LENGTH = 8,

    //! @brief EEPROM payload readerror
    BC_SOIL_SENSOR_ERROR_EEPROM_PAYLOAD_READ = 9,

    //! @brief EEPROM payload crc error
    BC_SOIL_SENSOR_ERROR_EEPROM_PAYLOAD_CRC = 10,

    //! @brief Zssc3123 inicialize error
    BC_SOIL_SENSOR_ERROR_ZSSC3123_INITIALIZE = 11,

    //! @brief Tmp112 measurement request
    BC_SOIL_SENSOR_ERROR_TMP112_MEASUREMENT_REQUEST = 12,

    //! @brief Zssc3123 measurement request
    BC_SOIL_SENSOR_ERROR_ZSSC3123_MEASUREMENT_REQUEST = 13,

    //! @brief Tmp112 data fetch
    BC_SOIL_SENSOR_ERROR_TMP112_DATA_FETCH = 14,

    //! @brief Zssc3123 data fetch
    BC_SOIL_SENSOR_ERROR_ZSSC3123_DATA_FETCH = 15

} bc_soil_sensor_error_t;

//! @brief Soil sensor instance

typedef struct bc_soil_sensor_t bc_soil_sensor_t;

typedef struct bc_soil_sensor_sensor_t bc_soil_sensor_sensor_t;

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

} bc_soil_sensor_eeprom_header_t;

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
    void (*_event_handler)(bc_soil_sensor_t *, uint64_t, bc_soil_sensor_event_t, void *);
    void *_event_param;

    bc_gpio_channel_t _channel;
    bc_soil_sensor_sensor_t *_sensor;
    int _sensor_count;
    int _sensor_found;
    bc_soil_sensor_error_t _error;
};

struct bc_soil_sensor_sensor_t
{
    bc_ds28e17_t _ds28e17;
    bool _temperature_valid;
    int16_t _temperature_raw;
    bool _cap_valid;
    uint16_t _cap_raw;
    bc_soil_sensor_eeprom_t _eeprom;
};

//! @endcond

//! @brief Initialize Soil sensor
//! @param[in] self Instance

void bc_soil_sensor_init(bc_soil_sensor_t *self);

//! @brief Initialize multiple Soil sensor
//! @param[in] self Instance
//! @param[in] sensors Sensors array
//! @param[in] sensor_count Sensors count

void bc_soil_sensor_init_multiple(bc_soil_sensor_t *self, bc_soil_sensor_sensor_t *sensors, int sensor_count);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_soil_sensor_set_event_handler(bc_soil_sensor_t *self, void (*event_handler)(bc_soil_sensor_t *, uint64_t, bc_soil_sensor_event_t, void *), void *event_param);

//! @brief Set measurement interval
//! @param[in] self Instance
//! @param[in] interval Measurement interval

void bc_soil_sensor_set_update_interval(bc_soil_sensor_t *self, bc_tick_t interval);

//! @brief Get sensors found
//! @param[in] self Instance
//! @return Number od found sensors

int bc_soil_sensor_get_sensor_found(bc_soil_sensor_t *self);

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
//! @return -1 Means no index for device address
//! @return > 0 Is valid index

int bc_soil_sensor_get_index_by_device_address(bc_soil_sensor_t *self, uint64_t device_address);

//! @brief Get device device address by its index
//! @param[in] self Instance
//! @param[in] index Index
//! @return 0 Means no device address for index
//! @return > 0 Is valid device address

uint64_t bc_soil_sensor_get_device_address_by_index(bc_soil_sensor_t *self, int index);

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

//! @brief Save calibration points and label to eeprom by device address
//! @param[in] self Instance
//! @param[in] device_address 64b device address

bool bc_soil_sensor_eeprom_save(bc_soil_sensor_t *self, uint64_t device_address);

//! @brief Get error number
//! @param[in] self Instance

bc_soil_sensor_error_t bc_soil_sensor_get_error(bc_soil_sensor_t *self);

//! @}

#endif // _BC_SOIL_SENSOR_H
