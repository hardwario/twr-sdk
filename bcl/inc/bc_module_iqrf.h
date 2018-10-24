#ifndef _BC_MODULE_IQRF_H
#define _BC_MODULE_IQRF_H


//! @addtogroup bc_module_iqrf bc_module_iqrf
//! @brief Driver for IQRF Module
//! @{

//! @brief Initialize button
//! @param[in] self Instance
//! @param[in] gpio_channel GPIO channel button is connected to
//! @param[in] gpio_pull GPIO pull-up/pull-down setting
//! @param[in] idle_state GPIO pin idle state (when button is not pressed)

bool bc_module_iqrf_init();


//! @}

#endif // _BC_MODULE_IQRF_H
