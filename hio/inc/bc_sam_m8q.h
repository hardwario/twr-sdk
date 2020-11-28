#ifndef _HIO_SAM_M8Q
#define _HIO_SAM_M8Q

#include <hio_i2c.h>
#include <hio_scheduler.h>

//! @addtogroup hio_sam_m8q hio_sam_m8q
//! @brief Driver for u-blox SAM-M8Q GPS/Galileo/Glonass navigation module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    HIO_SAM_M8Q_EVENT_ERROR = 0,

    //! @brief Start event
    HIO_SAM_M8Q_EVENT_START = 1,

    //! @brief Update event
    HIO_SAM_M8Q_EVENT_UPDATE = 2,

    //! @brief Stop event
    HIO_SAM_M8Q_EVENT_STOP = 3

} hio_sam_m8q_event_t;

//! @brief SAM-M8Q instance

typedef struct hio_sam_m8q_t hio_sam_m8q_t;

//! @brief SAM-M8Q driver

typedef struct
{
    //! @brief Callback for power on
    bool (*on)(hio_sam_m8q_t *self);

    //! @brief Callback for power off
    bool (*off)(hio_sam_m8q_t *self);

} hio_sam_m8q_driver_t;

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

} hio_sam_m8q_time_t;

//! @brief Position data structure

typedef struct
{
    //! @brief Latitude
    float latitude;

    //! @brief Longitude
    float longitude;

} hio_sam_m8q_position_t;

//! @brief Altitude data structure

typedef struct
{
    //! @brief Altitude
    float altitude;

    //! @brief Units of altitude
    char units;

} hio_sam_m8q_altitude_t;

//! @brief Quality data structure

typedef struct
{
    //! @brief Fix quality
    int fix_quality;

    //! @brief Number of satellites tracked
    int satellites_tracked;

} hio_sam_m8q_quality_t;

//! @brief Accuracy data structure

typedef struct
{
    //! @brief Horizontal accuracy estimate
    float horizontal;

    //! @brief Vertical accuracy estimate
    float vertical;

} hio_sam_m8q_accuracy_t;

//! @cond

typedef enum
{
    HIO_SAM_M8Q_STATE_ERROR = -1,
    HIO_SAM_M8Q_STATE_START = 0,
    HIO_SAM_M8Q_STATE_READ = 1,
    HIO_SAM_M8Q_STATE_UPDATE = 2,
    HIO_SAM_M8Q_STATE_STOP = 3

} hio_sam_m8q_state_t;

typedef void (hio_sam_m8q_event_handler_t)(hio_sam_m8q_t *, hio_sam_m8q_event_t, void *);

struct hio_sam_m8q_t
{
    hio_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    const hio_sam_m8q_driver_t *_driver;
    hio_scheduler_task_id_t _task_id;
    hio_sam_m8q_event_handler_t *_event_handler;
    void *_event_param;
    bool _running;
    bool _configured;
    hio_sam_m8q_state_t _state;
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

void hio_sam_m8q_init(hio_sam_m8q_t *self, hio_i2c_channel_t channel, uint8_t i2c_address, const hio_sam_m8q_driver_t *driver);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_sam_m8q_set_event_handler(hio_sam_m8q_t *self, hio_sam_m8q_event_handler_t event_handler, void *event_param);

//! @brief Start navigation module
//! @param[in] self Instance

void hio_sam_m8q_start(hio_sam_m8q_t *self);

//! @brief Stop navigation module
//! @param[in] self Instance

void hio_sam_m8q_stop(hio_sam_m8q_t *self);

//! @brief Invalidate navigation data

void hio_sam_m8q_invalidate(hio_sam_m8q_t *self);

//! @brief Get time
//! @param[in] self Instance
//! @param[out] time Time data structure
//! @return true On success
//! @return false On failure

bool hio_sam_m8q_get_time(hio_sam_m8q_t *self, hio_sam_m8q_time_t *time);

//! @brief Get position
//! @param[in] self Instance
//! @param[out] position Position data structure
//! @return true On success
//! @return false On failure

bool hio_sam_m8q_get_position(hio_sam_m8q_t *self, hio_sam_m8q_position_t *position);

//! @brief Get altitude
//! @param[in] self Instance
//! @param[out] altitude Altitude data structure
//! @return true On success
//! @return false On failure

bool hio_sam_m8q_get_altitude(hio_sam_m8q_t *self, hio_sam_m8q_altitude_t *altitude);

//! @brief Get quality
//! @param[in] self Instance
//! @param[out] quality Quality data structure
//! @return true On success
//! @return false On failure

bool hio_sam_m8q_get_quality(hio_sam_m8q_t *self, hio_sam_m8q_quality_t *quality);

//! @brief Get accuracy
//! @param[in] self Instance
//! @param[out] accuracy Accuracy data structure
//! @return true On success
//! @return false On failure

bool hio_sam_m8q_get_accuracy(hio_sam_m8q_t *self, hio_sam_m8q_accuracy_t *accuracy);

#endif // _HIO_SAM_M8Q
