#ifndef _BC_ADC_H
#define _BC_ADC_H

#include <bc_common.h>

//! @addtogroup bc_adc bc_adc
//! @brief Driver for ADC (analog to digital converter)
//! @{

//! @brief ADC channel

typedef enum
{
    BC_ADC_CHANNEL_A0 = 0L, //!< ADC channel A0
    BC_ADC_CHANNEL_A1 = 1L, //!< ADC channel A1
    BC_ADC_CHANNEL_A2 = 2L, //!< ADC channel A2
    BC_ADC_CHANNEL_A3 = 3L, //!< ADC channel A3
    BC_ADC_CHANNEL_A4 = 4L, //!< ADC channel A4
    BC_ADC_CHANNEL_A5 = 5L  //!< ADC channel A5

} bc_adc_channel_t;

//! @brief ADC voltage reference

typedef enum
{
    BC_ADC_REFERENCE_DEFAULT  = 0L, //!< ADC voltage reference is default (from VDD)
    BC_ADC_REFERENCE_INTERNAL = 1L  //!< ADC voltage reference is internal reference source

} bc_adc_reference_t;

//! @brief ADC result format

typedef enum
{
    BC_ADC_FORMAT_8_BIT  = 0L,  //!< ADC result format is 8-bit
    BC_ADC_FORMAT_16_BIT = 1L,  //!< ADC result format is 16-bit
    BC_ADC_FORMAT_24_BIT = 2L,  //!< ADC result format is 24-bit
    BC_ADC_FORMAT_32_BIT = 3L   //!< ADC result format is 32-bit
    // TODO ... BC_ADC_FORMAT_FLOAT_BIT = 4L//!< ADC result format is float

} bc_adc_format_t;

//! @brief Initialize ADC channel
//! @param[in] channel ADC channel
//! @param[in] reference ADC voltage reference
//! @param[in] format ADC result format

void bc_adc_init(bc_adc_channel_t channel, bc_adc_reference_t reference, bc_adc_format_t format);

//! @brief Set ADC voltage reference
//! @param[in] channel ADC channel
//! @param[in] reference ADC voltage reference

void bc_adc_set_reference(bc_adc_channel_t channel, bc_adc_reference_t reference);

//! @brief Get ADC voltage reference
//! @param[in] channel ADC channel
//! @return ADC voltage reference

bc_adc_reference_t bc_adc_get_reference(bc_adc_channel_t channel);

//! @brief Set ADC result format
//! @param[in] channel ADC channel
//! @param[in] format ADC result format

void bc_adc_set_format(bc_adc_channel_t channel, bc_adc_format_t format);

//! @brief Get ADC result format
//! @param[in] channel ADC channel
//! @return ADC result format

bc_adc_format_t bc_adc_get_format(bc_adc_channel_t channel);

//! @brief Perform measurement on ADC channel
//! @param[in] channel ADC channel
//! @param[out] result Pointer to destination where ADC conversion will be stored

void bc_adc_measure(bc_adc_channel_t channel, void *result);

//! @}

#endif // _BC_ADC_H
