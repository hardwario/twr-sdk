#ifndef _BC_ADC_H
#define _BC_ADC_H

#include <bc_common.h>

//! @addtogroup bc_adc bc_adc
//! @brief Driver for ADC (analog to digital converter)
//! @{

//! @brief ADC channel

typedef enum
{
    //! @brief ADC channel A0
    BC_ADC_CHANNEL_A0 = 0,

    //! @brief ADC channel A1
    BC_ADC_CHANNEL_A1 = 1,

    //! @brief ADC channel A2
    BC_ADC_CHANNEL_A2 = 2,

    //! @brief ADC channel A3
    BC_ADC_CHANNEL_A3 = 3,

    //! @brief ADC channel A4
    BC_ADC_CHANNEL_A4 = 4,

    //! @brief ADC channel A5
    BC_ADC_CHANNEL_A5 = 5

} bc_adc_channel_t;

//! @brief ADC result format

typedef enum
{
    //! @brief ADC result format is 8-bit
    BC_ADC_FORMAT_8_BIT  = 0,

    //! @brief ADC result format is 16-bit
    BC_ADC_FORMAT_16_BIT = 1,

    //! @brief ADC result format is 24-bit
    BC_ADC_FORMAT_24_BIT = 2,

    //! @brief ADC result format is 32-bit
    BC_ADC_FORMAT_32_BIT = 3,

    //! @brief ADC result format is float
    BC_ADC_FORMAT_FLOAT = 4

} bc_adc_format_t;

//! @brief ADC event

typedef enum
{
    //! @brief ADC event
    BC_ADC_EVENT_DONE

} bc_adc_event_t;

//! @brief Initialize ADC channel
//! @param[in] channel ADC channel
//! @param[in] format ADC result format

void bc_adc_init(bc_adc_channel_t channel, bc_adc_format_t format);

//! @brief Set ADC result format
//! @param[in] channel ADC channel
//! @param[in] format ADC result format

void bc_adc_set_format(bc_adc_channel_t channel, bc_adc_format_t format);

//! @brief Get ADC result format
//! @param[in] channel ADC channel
//! @return ADC result format

bc_adc_format_t bc_adc_get_format(bc_adc_channel_t channel);

//! @brief Reads the ADC channel voltage
//! @param[in] channel ADC channel
//! @param[out] result Pointer to destination where ADC conversion will be stored
//! @return true On success
//! @return false On failure

bool bc_adc_read(bc_adc_channel_t channel, void *result);

//! @brief Set callback function
//! @param[in] channel ADC channel
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)
//! @return true On success
//! @return false On failure

bool bc_adc_set_event_handler(bc_adc_channel_t channel, void (*event_handler)(bc_adc_channel_t, bc_adc_event_t, void *), void *event_param);

//! @brief Begins reading the ADC channel voltage in asynchronous mode
//! @param[in] channel ADC channel
//! @return true On success
//! @return false On failure

bool bc_adc_async_read(bc_adc_channel_t channel);

//! @brief Get measurement result
//! @param[in] channel ADC channel
//! @param[out] result Pointer to variable where result will be stored
//! @return true On success
//! @return false On failure

bool bc_adc_get_result(bc_adc_channel_t channel, void *result);

//! @brief Get voltage on VDDA pin
//! @param[out] vdda_voltage Pointer to destination where VDDA will be stored
//! @return true On valid VDDA
//! @return false On valid VDDA

bool bc_adc_get_vdda_voltage(float *vdda_voltage);

//! @}

#endif // _BC_ADC_H
