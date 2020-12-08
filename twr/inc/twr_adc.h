#ifndef _TWR_ADC_H
#define _TWR_ADC_H

#include <twr_common.h>
#include <stm32l083xx.h>

//! @addtogroup twr_adc twr_adc
//! @brief Driver for ADC (analog to digital converter)
//! @{

//! @brief ADC channel

typedef enum
{
    //! @brief ADC channel A0
    TWR_ADC_CHANNEL_A0 = 0,

    //! @brief ADC channel A1
    TWR_ADC_CHANNEL_A1 = 1,

    //! @brief ADC channel A2
    TWR_ADC_CHANNEL_A2 = 2,

    //! @brief ADC channel A3
    TWR_ADC_CHANNEL_A3 = 3,

    //! @brief ADC channel A4
    TWR_ADC_CHANNEL_A4 = 4,

    //! @brief ADC channel A5
    TWR_ADC_CHANNEL_A5 = 5,

    //! @brief ADC channel A6
    TWR_ADC_CHANNEL_A6 = 6

} twr_adc_channel_t;

//! @brief ADC oversampling

typedef enum
{
    //! @brief ADC no oversampling
    TWR_ADC_OVERSAMPLING_NONE  = 0,

    //! @brief ADC 2x oversampling
    TWR_ADC_OVERSAMPLING_2 = 1,

    //! @brief ADC 4x oversampling
    TWR_ADC_OVERSAMPLING_4 = 2,

    //! @brief ADC 8x oversampling
    TWR_ADC_OVERSAMPLING_8 = 3,

    //! @brief ADC 16x oversampling
    TWR_ADC_OVERSAMPLING_16 = 4,

    //! @brief ADC 32x oversampling
    TWR_ADC_OVERSAMPLING_32 = 5,

    //! @brief ADC 64x oversampling
    TWR_ADC_OVERSAMPLING_64 = 6,

    //! @brief ADC 128x oversampling
    TWR_ADC_OVERSAMPLING_128 = 7,

    //! @brief ADC 256x oversampling
    TWR_ADC_OVERSAMPLING_256 = 8,

} twr_adc_oversampling_t;

//! @brief ADC resolution

typedef enum
{
    //! @brief ADC 12 bit resolution
    TWR_ADC_RESOLUTION_12_BIT  = 0,

    //! @brief ADC 10 bit resolution
    TWR_ADC_RESOLUTION_10_BIT = ADC_CFGR1_RES_0,

    //! @brief ADC 8 bit resolution
    TWR_ADC_RESOLUTION_8_BIT = ADC_CFGR1_RES_1,

    //! @brief ADC 6 bit resolution
    TWR_ADC_RESOLUTION_6_BIT = ADC_CFGR1_RES_1 | ADC_CFGR1_RES_0,

} twr_adc_resolution_t;

//! @brief ADC event

typedef enum
{
    //! @brief ADC event
    TWR_ADC_EVENT_DONE

} twr_adc_event_t;

//! @brief Initialize ADC converter

void twr_adc_init();

//! @brief Check if ADC  is ready for reading
//! @return true If ready
//! @return false If not ready

bool twr_adc_is_ready();

//! @brief Reads the ADC channel value
//! @param[in] channel ADC channel
//! @param[out] result Pointer to destination where ADC conversion will be stored
//! @return true On success
//! @return false On failure

bool twr_adc_get_value(twr_adc_channel_t channel, uint16_t *result);

//! @brief Set callback function
//! @param[in] channel ADC channel
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)
//! @return true On success
//! @return false On failure

bool twr_adc_set_event_handler(twr_adc_channel_t channel, void (*event_handler)(twr_adc_channel_t, twr_adc_event_t, void *), void *event_param);

//! @brief Begins reading the ADC channel voltage in asynchronous mode
//! @param[in] channel ADC channel
//! @return true On success
//! @return false On failure

bool twr_adc_async_measure(twr_adc_channel_t channel);

//! @brief Get asynchronous measurement result
//! @param[in] channel ADC channel
//! @param[out] result Pointer to variable where result will be stored
//! @return true On success
//! @return false On failure

bool twr_adc_async_get_value(twr_adc_channel_t channel, uint16_t *result);

//! @brief Get asynchronous measurement result in volts
//! @param[in] channel ADC channel
//! @param[out] result Pointer to variable where result in volts will be stored
//! @return true On success
//! @return false On failure

bool twr_adc_async_get_voltage(twr_adc_channel_t channel, float *result);

//! @brief Get voltage on VDDA pin
//! @param[out] vdda_voltage Pointer to destination where VDDA will be stored
//! @return true On valid VDDA
//! @return false On valid VDDA

bool twr_adc_get_vdda_voltage(float *vdda_voltage);

//! @brief Calibration
//! @return true On success
//! @return false On failure

bool twr_adc_calibration(void);

//! @brief Set ADC resolution for specific channel
//! @param[in] channel ADC channel
//! @param[in] resolution Resolution can be 6, 8, 10 or 12 bit

void twr_adc_resolution_set(twr_adc_channel_t channel, twr_adc_resolution_t resolution);

//! @brief Set ADC oversampling for specific channel
//! @param[in] channel ADC channel
//! @param[in] oversampling Oversampling can be 2, 4, 8, 16, 32, 64, 128 or 256

void twr_adc_oversampling_set(twr_adc_channel_t channel, twr_adc_oversampling_t oversampling);

//! @}

#endif // _TWR_ADC_H
