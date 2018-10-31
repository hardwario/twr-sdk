#ifndef _BC_MODULE_IQRF_H
#define _BC_MODULE_IQRF_H

#define int8 int8_t
#define int16 int16_t
#define uns8 uint8_t
#define uns16 uint16_t
#define byte uint8_t

#include "DPA.h"
#include "IQRF_HWPID.h"
#include "IQRFstandard.h"

//! @addtogroup bc_module_iqrf bc_module_iqrf
//! @brief Driver for IQRF Module
//! @{

//! @brief Callback events

typedef enum
{
    //! @brief
    BC_MODULE_IQRF_EVENT_PERIPHERAL_REQUEST = 0,

    //! @brief
    BC_MODULE_IQRF_EVENT_PERIPHERAL_INFO_REQUEST = 1,

    //! @brief
    BC_MODULE_IQRF_EVENT_PCMD_STD_ENUMERATE = 2,

    //! @brief
    BC_MODULE_IQRF_EVENT_PCMD_STD_SENSORS_READ_VALUES = 3,

    //! @brief
    BC_MODULE_IQRF_EVENT_PCMD_STD_SENSORS_READ_TYPES_AND_VALUES = 4

} bc_module_iqrf_event_t;


//! @cond

typedef struct bc_module_iqrf_t bc_module_iqrf_t;

struct bc_module_iqrf_t
{
    void (*_event_handler)(bc_module_iqrf_t *, bc_module_iqrf_event_t, void *);
    void *_event_param;
    bc_scheduler_task_id_t _task_id;

    TDpaMessage *dpa_message;

    uint8_t return_flags;
    uint8_t return_data_length;
    uint8_t return_dpa_data_length;
};

//! @endcond

//! @brief Initialize button
//! @param[in] self Instance
//! @param[in] gpio_channel GPIO channel button is connected to
//! @param[in] gpio_pull GPIO pull-up/pull-down setting
//! @param[in] idle_state GPIO pin idle state (when button is not pressed)

bool bc_module_iqrf_init();

void bc_module_iqrf_set_event_handler(void (*event_handler)(bc_module_iqrf_t *, bc_module_iqrf_event_t, void *), void *event_param);


//! @}

#endif // _BC_MODULE_IQRF_H
