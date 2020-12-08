#ifndef _TWR_SAM_M8Q
#define _TWR_SAM_M8Q

#include <twr_i2c.h>
#include <twr_scheduler.h>

//! @addtogroup twr_sam_m8q twr_sam_m8q
//! @brief Driver for u-blox SAM-M8Q GPS/Galileo/Glonass navigation module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    TWR_SAM_M8Q_EVENT_ERROR = 0,

    //! @brief Start event
    TWR_SAM_M8Q_EVENT_START = 1,

    //! @brief Update event
    TWR_SAM_M8Q_EVENT_UPDATE = 2,

    //! @brief Stop event
    TWR_SAM_M8Q_EVENT_STOP = 3

} twr_sam_m8q_event_t;

//! @brief SAM-M8Q instance

typedef struct twr_sam_m8q_t twr_sam_m8q_t;

//! @brief SAM-M8Q driver

typedef struct
{
    //! @brief Callback for power on
    bool (*on)(twr_sam_m8q_t *self);

    //! @brief Callback for power off
    bool (*off)(twr_sam_m8q_t *self);

} twr_sam_m8q_driver_t;

//! @brief Time data structure

typedef struct
{
    //! @brief Year
    int year;

    //! @brief Month
    int month;

    //! @brief Day
    int day;

    //! @brief Hours
    int hours;

    //! @brief Minutes
    int minutes;

    //! @brief Seconds
    int seconds;

} twr_sam_m8q_time_t;

//! @brief Position data structure

typedef struct
{
    //! @brief Latitude
    float latitude;

    //! @brief Longitude
    float longitude;

} twr_sam_m8q_position_t;

//! @brief Altitude data structure

typedef struct
{
    //! @brief Altitude
    float altitude;

    //! @brief Units of altitude
    char units;

} twr_sam_m8q_altitude_t;

//! @brief Quality data structure

typedef struct
{
    //! @brief Fix quality
    int fix_quality;

    //! @brief Number of satellites tracked
    int satellites_tracked;

} twr_sam_m8q_quality_t;

//! @brief Accuracy data structure

typedef struct
{
    //! @brief Horizontal accuracy estimate
    float horizontal;

    //! @brief Vertical accuracy estimate
    float vertical;

} twr_sam_m8q_accuracy_t;

//! @cond

typedef enum
{
    TWR_SAM_M8Q_STATE_ERROR = -1,
    TWR_SAM_M8Q_STATE_START = 0,
    TWR_SAM_M8Q_STATE_READ = 1,
    TWR_SAM_M8Q_STATE_UPDATE = 2,
    TWR_SAM_M8Q_STATE_STOP = 3

} twr_sam_m8q_state_t;

typedef void (twr_sam_m8q_event_handler_t)(twr_sam_m8q_t *, twr_sam_m8q_event_t, void *);

struct twr_sam_m8q_t
{
    twr_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    const twr_sam_m8q_driver_t *_driver;
    twr_scheduler_task_id_t _task_id;
    twr_sam_m8q_event_handler_t *_event_handler;
    void *_event_param;
    bool _running;
    bool _configured;
    twr_sam_m8q_state_t _state;
    uint8_t _ddc_buffer[64];
    size_t _ddc_length;
    char _line_buffer[128];
    size_t _line_length;
    bool _line_clipped;
    struct
    {
        bool valid;
        struct
        {
            int year;
            int month;
            int day;
        } date;
        struct
        {
            int hours;
            int minutes;
            int seconds;
        } time;
        float latitude;
        float longitude;
    } _rmc;
    struct
    {
        bool valid;
        int fix_quality;
        float altitude;
        char altitude_units;
    } _gga;
    struct
    {
        bool valid;
        float h_accuracy;
        float v_accuracy;
        float speed;
        float course;
        int satellites;
    } _pubx;
};

//! @endcond

//! @brief Initialize SAM-M8Q module driver
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address
//! @param[in] driver Optional driver (can be NULL)

void twr_sam_m8q_init(twr_sam_m8q_t *self, twr_i2c_channel_t channel, uint8_t i2c_address, const twr_sam_m8q_driver_t *driver);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_sam_m8q_set_event_handler(twr_sam_m8q_t *self, twr_sam_m8q_event_handler_t event_handler, void *event_param);

//! @brief Start navigation module
//! @param[in] self Instance

void twr_sam_m8q_start(twr_sam_m8q_t *self);

//! @brief Stop navigation module
//! @param[in] self Instance

void twr_sam_m8q_stop(twr_sam_m8q_t *self);

//! @brief Invalidate navigation data

void twr_sam_m8q_invalidate(twr_sam_m8q_t *self);

//! @brief Get time
//! @param[in] self Instance
//! @param[out] time Time data structure
//! @return true On success
//! @return false On failure

bool twr_sam_m8q_get_time(twr_sam_m8q_t *self, twr_sam_m8q_time_t *time);

//! @brief Get position
//! @param[in] self Instance
//! @param[out] position Position data structure
//! @return true On success
//! @return false On failure

bool twr_sam_m8q_get_position(twr_sam_m8q_t *self, twr_sam_m8q_position_t *position);

//! @brief Get altitude
//! @param[in] self Instance
//! @param[out] altitude Altitude data structure
//! @return true On success
//! @return false On failure

bool twr_sam_m8q_get_altitude(twr_sam_m8q_t *self, twr_sam_m8q_altitude_t *altitude);

//! @brief Get quality
//! @param[in] self Instance
//! @param[out] quality Quality data structure
//! @return true On success
//! @return false On failure

bool twr_sam_m8q_get_quality(twr_sam_m8q_t *self, twr_sam_m8q_quality_t *quality);

//! @brief Get accuracy
//! @param[in] self Instance
//! @param[out] accuracy Accuracy data structure
//! @return true On success
//! @return false On failure

bool twr_sam_m8q_get_accuracy(twr_sam_m8q_t *self, twr_sam_m8q_accuracy_t *accuracy);

#endif // _TWR_SAM_M8Q
