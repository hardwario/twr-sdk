#ifndef _BC_MODULE_SIGFOX_H
#define _BC_MODULE_SIGFOX_H

#include <bc_td1207r.h>
#include <bc_wssfm10r1at.h>

//! @addtogroup bc_module_sigfox bc_module_sigfox
//! @brief Driver for BigClown SigFox Module
//! @{

//! @brief SigFox Module hardware revision

typedef enum
{
    //! @brief Hardware revision R1
    BC_MODULE_SIGFOX_REVISION_R1 = 0,

    //! @brief Hardware revision R2
    BC_MODULE_SIGFOX_REVISION_R2 = 1

} bc_module_sigfox_revision_t;

//! @brief Callback events

typedef enum
{
    //! @brief Ready event
    BC_MODULE_SIGFOX_EVENT_READY = 0,

    //! @brief Error event
    BC_MODULE_SIGFOX_EVENT_ERROR = 1,

    //! @brief RF frame transmission started event
    BC_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_START = 2,

    //! @brief RF frame transmission finished event
    BC_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_DONE = 3,

    //! @brief Device ID has been read event
    BC_MODULE_SIGFOX_EVENT_READ_DEVICE_ID = 4,

    //! @brief Device PAC has been read event
    BC_MODULE_SIGFOX_EVENT_READ_DEVICE_PAC = 5

} bc_module_sigfox_event_t;

//! @cond

typedef struct bc_module_sigfox_t bc_module_sigfox_t;

struct bc_module_sigfox_t
{
    bc_module_sigfox_revision_t _revision;
    void (*_event_handler)(bc_module_sigfox_t *, bc_module_sigfox_event_t, void *);
    void *_event_param;
    union
    {
        bc_td1207r_t td1207r;
        bc_wssfm10r1at_t wssfm10r1at;
    } _modem;
};

//! @endcond

//! @brief Initialize BigClown SigFox Module
//! @param[in] self Instance

void bc_module_sigfox_init(bc_module_sigfox_t *self, bc_module_sigfox_revision_t revision);

//! @brief Set callback function
//! @param[in] self Instance
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_module_sigfox_set_event_handler(bc_module_sigfox_t *self, void (*event_handler)(bc_module_sigfox_t *, bc_module_sigfox_event_t, void *), void *event_param);

//! @brief Check if modem is ready for commands
//! @param[in] self Instance
//! @return true if ready
//! @return false if not ready

bool bc_module_sigfox_is_ready(bc_module_sigfox_t *self);

//! @brief Send RF frame command
//! @param[in] self Instance
//! @param[in] buffer Pointer to data to be transmitted
//! @param[in] length Length of data to be transmitted in bytes (must be from 1 to 12 bytes)
//! @return true if command was accepted for processing
//! @return false if command was denied for processing

bool bc_module_sigfox_send_rf_frame(bc_module_sigfox_t *self, const void *buffer, size_t length);

//! @brief Read device ID command
//! @param[in] self Instance
//! @return true if command was accepted for processing
//! @return false if command was denied for processing

bool bc_module_sigfox_read_device_id(bc_module_sigfox_t *self);

//! @brief Read device PAC command
//! @param[in] self Instance
//! @return true if command was accepted for processing
//! @return false if command was denied for processing

bool bc_module_sigfox_read_device_pac(bc_module_sigfox_t *self);

//! @brief Generate continuous wave command
//! @param[in] self Instance
//! @return true if command was accepted for processing
//! @return false if command was denied for processing

bool bc_module_sigfox_continuous_wave(bc_module_sigfox_t *self);

//! @}

#endif // _BC_MODULE_SIGFOX_H
