#ifndef _BC_SAM_M8
#define _BC_SAM_M8

#include <bc_i2c.h>
#include <bc_scheduler.h>

//! @addtogroup bc_sam_m8 bc_sam_m8
//! @brief Driver for SAM M8 module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief Error event
    BC_SAM_M8_EVENT_ERROR = 0,

    //! @brief Start event
    BC_SAM_M8_EVENT_START = 1,

    //! @brief Stop event
    BC_SAM_M8_EVENT_STOP = 2,

    //! @brief Update event
    BC_SAM_M8_EVENT_UPDATE = 3,
    
    //! @brief Timeout event
    BC_SAM_M8_EVENT_TIMEOUT = 4

} bc_sam_m8_event_t;

//! @brief States

typedef enum
{
    BC_SAM_M8_STATE_ERROR = -1,

    BC_SAM_M8_STATE_START = 0,

    BC_SAM_M8_STATE_STOP = 1,
    
    BC_SAM_M8_STATE_READ = 2,
    
    BC_SAM_M8_STATE_UPDATE = 3,
    
    BC_SAM_M8_STATE_TIMEOUT = 4

} bc_sam_m8_state_t;

//! @brief SAM M8 instance

typedef struct bc_sam_m8_t bc_sam_m8_t;

//! @cond

typedef void (bc_sam_m8_event_handler_t)(bc_sam_m8_t *, bc_sam_m8_event_t, void *);

struct bc_sam_m8_t
{
    bool _running;

    uint8_t _ddc_buffer[64];

    size_t _ddc_length;

    char _line_buffer[128];

    size_t _line_length;

    bool _line_clipped;

    bc_sam_m8_state_t _state;

    uint64_t _gnss_nmea_sentences;

    uint64_t _nb_boot_attempts;

    float _latitude;

    float _longitude;

    bc_scheduler_task_id_t _task_id;

    bc_i2c_channel_t _i2c_channel;

    uint8_t _i2c_address;

    // ------------------------------------------------------------------
    // TODO: Consider using of virtual pin / add to bc_sam_m8_init
    // ------------------------------------------------------------------
    uint8_t _i2c_address_expander;

    uint8_t _expander_pin;
    // ------------------------------------------------------------------

    bc_sam_m8_event_handler_t *_event_handler;

    void *_event_param;

    bc_tick_t _timeout;
};

//! @endcond

//! @brief Initialize SAM M8Q driver
//! @param[in] self Instance
//! @param[in] i2c_channel I2C channel
//! @param[in] i2c_address I2C device address
//! @param[in] i2c_address_expander I2C device expander address
//! @param[in] expander_pin I2C device expander pin

void bc_sam_m8_init(bc_sam_m8_t *self, bc_i2c_channel_t channel, uint8_t i2c_address, uint8_t i2c_address_expander, uint8_t expander_pin);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_sam_m8_set_event_handler(bc_sam_m8_t *self, bc_sam_m8_event_handler_t event_handler, void *event_param);

//! @brief Start GPS
//! @param[in] self Instance
//! @param[in] milliseconds Timeout interval in milliseconds

void bc_sam_m8_start(bc_sam_m8_t *self, uint64_t milliseconds);

//! @brief Stop GPS
//! @param[in] self Instance

void bc_sam_m8_stop(bc_sam_m8_t *self);

//! @brief Get GPS position
//! @param[in] self Instance
//! @param[out] Latitude latitude
//! @param[out] Longitude longitude
//! @return true On success
//! @return false On failure

bool bc_sam_m8_get_position(bc_sam_m8_t *self, float *latitude, float *longitude);

#endif // _BC_SAM_M8
