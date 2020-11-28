#ifndef _HIO_MODULE_SIGFOX_H
#define _HIO_MODULE_SIGFOX_H

#include <hio_td1207r.h>
#include <hio_wssfm10r1at.h>

//! @addtogroup hio_module_sigfox hio_module_sigfox
//! @brief Driver for HARDWARIO SigFox Module
//! @{

//! @brief SigFox Module hardware revision

typedef enum
{
    //! @brief Hardware revision R1
    HIO_MODULE_SIGFOX_REVISION_R1 = 0,

    //! @brief Hardware revision R2
    HIO_MODULE_SIGFOX_REVISION_R2 = 1

} hio_module_sigfox_revision_t;

//! @brief Callback events

typedef enum
{
    //! @brief Ready event
    HIO_MODULE_SIGFOX_EVENT_READY = 0,

    //! @brief Error event
    HIO_MODULE_SIGFOX_EVENT_ERROR = 1,

    //! @brief RF frame transmission started event
    HIO_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_START = 2,

    //! @brief RF frame transmission finished event
    HIO_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_DONE = 3,

    //! @brief Device ID has been read event
    HIO_MODULE_SIGFOX_EVENT_READ_DEVICE_ID = 4,

    //! @brief Device PAC has been read event
    HIO_MODULE_SIGFOX_EVENT_READ_DEVICE_PAC = 5

} hio_module_sigfox_event_t;

//! @cond

typedef struct hio_module_sigfox_t hio_module_sigfox_t;

struct hio_module_sigfox_t
{
    hio_module_sigfox_revision_t _revision;
    void (*_event_handler)(hio_module_sigfox_t *, hio_module_sigfox_event_t, void *);
    void *_event_param;
    union
    {
        hio_td1207r_t td1207r;
        hio_wssfm10r1at_t wssfm10r1at;
    } _modem;
};

//! @endcond

//! @brief Initialize HARDWARIO SigFox Module
//! @param[in] self Instance

void hio_module_sigfox_init(hio_module_sigfox_t *self, hio_module_sigfox_revision_t revision);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void hio_module_sigfox_set_event_handler(hio_module_sigfox_t *self, void (*event_handler)(hio_module_sigfox_t *, hio_module_sigfox_event_t, void *), void *event_param);

//! @brief Check if modem is ready for commands
//! @param[in] self Instance
//! @return true If ready
//! @return false If not ready

bool hio_module_sigfox_is_ready(hio_module_sigfox_t *self);

//! @brief Send RF frame command
//! @param[in] self Instance
//! @param[in] buffer Pointer to data to be transmitted
//! @param[in] length Length of data to be transmitted in bytes (must be from 1 to 12 bytes)
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool hio_module_sigfox_send_rf_frame(hio_module_sigfox_t *self, const void *buffer, size_t length);

//! @brief Read device ID command
//! @param[in] self Instance
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool hio_module_sigfox_read_device_id(hio_module_sigfox_t *self);

//! @brief Get device ID (can be called only in HIO_WSSFM10R1AT_EVENT_READ_DEVICE_ID event)
//! @param[in] self Instance
//! @param[out] buffer Pointer to destination buffer
//! @param[in] buffer_size Size of destination buffer
//! @return true If device ID was retrieved
//! @return false If device ID could not be retrieved

bool hio_module_sigfox_get_device_id(hio_module_sigfox_t *self, char *buffer, size_t buffer_size);

//! @brief Read device PAC command
//! @param[in] self Instance
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool hio_module_sigfox_read_device_pac(hio_module_sigfox_t *self);

//! @brief Get device PAC (can be called only in HIO_WSSFM10R1AT_EVENT_READ_DEVICE_ID event)
//! @param[in] self Instance
//! @param[out] buffer Pointer to destination buffer
//! @param[in] buffer_size Size of destination buffer
//! @return true If device PAC was retrieved
//! @return false If device PAC could not be retrieved

bool hio_module_sigfox_get_device_pac(hio_module_sigfox_t *self, char *buffer, size_t buffer_size);

//! @brief Generate continuous wave command
//! @param[in] self Instance
//! @return true If command was accepted for processing
//! @return false If command was denied for processing

bool hio_module_sigfox_continuous_wave(hio_module_sigfox_t *self);

//! @}

#endif // _HIO_MODULE_SIGFOX_H
