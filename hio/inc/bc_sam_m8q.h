#ifndef _BC_SAM_M8Q
#define _BC_SAM_M8Q

#include <bc_i2c.h>
#include <bc_scheduler.h>

//! @addtogroup bc_sam_m8q bc_sam_m8q
//! @brief Driver for u-blox SAM-M8Q GPS/Galileo/Glonass navigation module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_SAM_M8Q_EVENT_ERROR = 0,

    //! @brief Start event
    BC_SAM_M8Q_EVENT_START = 1,

    //! @brief Update event
    BC_SAM_M8Q_EVENT_UPDATE = 2,

    //! @brief Stop event
    BC_SAM_M8Q_EVENT_STOP = 3

} bc_sam_m8q_event_t;

//! @brief SAM-M8Q instance

typedef struct bc_sam_m8q_t bc_sam_m8q_t;

//! @brief SAM-M8Q driver

typedef struct
{
    //! @brief Callback for power on
    bool (*on)(bc_sam_m8q_t *self);

    //! @brief Callback for power off
    bool (*off)(bc_sam_m8q_t *self);

} bc_sam_m8q_driver_t;

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

} bc_sam_m8q_time_t;

//! @brief Position data structure

typedef struct
{
    //! @brief Latitude
    float latitude;

    //! @brief Longitude
    float longitude;

} bc_sam_m8q_position_t;

//! @brief Altitude data structure

typedef struct
{
    //! @brief Altitude
    float altitude;

    //! @brief Units of altitude
    char units;

} bc_sam_m8q_altitude_t;

//! @brief Quality data structure

typedef struct
{
    //! @brief Fix quality
    int fix_quality;

    //! @brief Number of satellites tracked
    int satellites_tracked;

} bc_sam_m8q_quality_t;

//! @brief Accuracy data structure

typedef struct
{
    //! @brief Horizontal accuracy estimate
    float horizontal;

    //! @brief Vertical accuracy estimate
    float vertical;

} bc_sam_m8q_accuracy_t;

//! @cond

typedef enum
{
    BC_SAM_M8Q_STATE_ERROR = -1,
    BC_SAM_M8Q_STATE_START = 0,
    BC_SAM_M8Q_STATE_READ = 1,
    BC_SAM_M8Q_STATE_UPDATE = 2,
    BC_SAM_M8Q_STATE_STOP = 3

} bc_sam_m8q_state_t;

typedef void (bc_sam_m8q_event_handler_t)(bc_sam_m8q_t *, bc_sam_m8q_event_t, void *);

struct bc_sam_m8q_t
{
    bc_i2c_channel_t _i2c_channel;
    uint8_t _i2c_address;
    const bc_sam_m8q_driver_t *_driver;
    bc_scheduler_task_id_t _task_id;
    bc_sam_m8q_event_handler_t *_event_handler;
    void *_event_param;
    bool _running;
    bool _configured;
    bc_sam_m8q_state_t _state;
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

void bc_sam_m8q_init(bc_sam_m8q_t *self, bc_i2c_channel_t channel, uint8_t i2c_address, const bc_sam_m8q_driver_t *driver);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_sam_m8q_set_event_handler(bc_sam_m8q_t *self, bc_sam_m8q_event_handler_t event_handler, void *event_param);

//! @brief Start navigation module
//! @param[in] self Instance

void bc_sam_m8q_start(bc_sam_m8q_t *self);

//! @brief Stop navigation module
//! @param[in] self Instance

void bc_sam_m8q_stop(bc_sam_m8q_t *self);

//! @brief Invalidate navigation data

void bc_sam_m8q_invalidate(bc_sam_m8q_t *self);

//! @brief Get time
//! @param[in] self Instance
//! @param[out] time Time data structure
//! @return true On success
//! @return false On failure

bool bc_sam_m8q_get_time(bc_sam_m8q_t *self, bc_sam_m8q_time_t *time);

//! @brief Get position
//! @param[in] self Instance
//! @param[out] position Position data structure
//! @return true On success
//! @return false On failure

bool bc_sam_m8q_get_position(bc_sam_m8q_t *self, bc_sam_m8q_position_t *position);

//! @brief Get altitude
//! @param[in] self Instance
//! @param[out] altitude Altitude data structure
//! @return true On success
//! @return false On failure

bool bc_sam_m8q_get_altitude(bc_sam_m8q_t *self, bc_sam_m8q_altitude_t *altitude);

//! @brief Get quality
//! @param[in] self Instance
//! @param[out] quality Quality data structure
//! @return true On success
//! @return false On failure

bool bc_sam_m8q_get_quality(bc_sam_m8q_t *self, bc_sam_m8q_quality_t *quality);

//! @brief Get accuracy
//! @param[in] self Instance
//! @param[out] accuracy Accuracy data structure
//! @return true On success
//! @return false On failure

bool bc_sam_m8q_get_accuracy(bc_sam_m8q_t *self, bc_sam_m8q_accuracy_t *accuracy);

#endif // _BC_SAM_M8Q
