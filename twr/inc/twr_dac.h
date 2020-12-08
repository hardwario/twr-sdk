#ifndef _TWR_DAC_H
#define _TWR_DAC_H

#include <twr_common.h>
#include <twr_dma.h>

//! @addtogroup twr_dac twr_dac
//! @brief Driver for DAC
//! @{

//! @brief DAC channel

typedef enum
{
    //! @brief DAC channel DAC0
    TWR_DAC_DAC0 = 0,

    //! @brief DAC channel DAC1
    TWR_DAC_DAC1 = 1

} twr_dac_channel_t;

//! @brief Raw value format

typedef enum
{
    //! @brief Raw value format is 8-bit
    TWR_DAC_FORMAT_8_BIT = 0,

    //! @brief Raw value format is 16-bit
    TWR_DAC_FORMAT_16_BIT = 1,

    //! @brief Raw value format is float
    TWR_DAC_FORMAT_VOLTAGE = 2

} twr_dac_format_t;

//! @brief Sample rate

typedef enum
{
    //! @brief Data sample-rate is 8kHz
    TWR_DAC_SAMPLE_RATE_8K = 0,

    //! @brief Data sample-rate is 16kHz
    TWR_DAC_SAMPLE_RATE_16K = 1

} twr_dac_sample_rate_t;

//! @brief Data size

typedef enum
{
    //! @brief Data size is 8b
    TWR_DAC_DATA_SIZE_8 = 0,

    //! @brief Data size is 16b
    TWR_DAC_DATA_SIZE_16 = 1

} twr_dac_data_size_t;

//! @brief Mode

typedef enum
{
    //! @brief Mode single
    TWR_DAC_MODE_SINGLE = 0,

    //! @brief Mode circular (repeate playing buffer)
    TWR_DAC_MODE_CIRCULAR = 1

} twr_dac_mode_t;

//! @brief DAC channel event

typedef enum
{
    //! @brief Event is done
    TWR_DAC_EVENT_HALF_DONE = 0,

    //! @brief Event is done
    TWR_DAC_EVENT_DONE = 1

} twr_dac_event_t;

//! @brief DAC channel event

typedef struct
{
    void *buffer;
    uint16_t length;
    twr_dac_data_size_t data_size;
    twr_dac_sample_rate_t sample_rate;
    twr_dac_mode_t mode;

} twr_dac_config_t;

//! @brief Initialize DAC channel
//! @param[in] channel DAC channel

void twr_dac_init(twr_dac_channel_t channel);

//! @brief Deitialize DAC channel
//! @param[in] channel DAC channel

void twr_dac_deinit(twr_dac_channel_t channel);

//! @brief Set DAC channel output as raw value
//! @param[in] channel DAC channel
//! @param[out] raw Pointer to desired raw value
//! @param[in] format Desired raw value format

void twr_dac_set_output(twr_dac_channel_t channel, const void *raw, twr_dac_format_t format);

//! @brief Set callback function
//! @param[in] channel DAC channel
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_dac_set_event_handler(twr_dac_channel_t channel, void (*event_handler)(twr_dac_channel_t, twr_dac_event_t, void *), void *event_param);

//! @brief Configure image of DAC DMA channel
//! @param[in] channel DMA channel
//! @param[in] buffer Data buffer
//! @param[in] length Data buffer length
//! @param[in] sample_rate Data sample rate
//! @param[in] mode DAC channel DMA mode
//! @return true On success
//! @return false On failure (if DAC channel operation is in progress)

bool twr_dac_async_config(twr_dac_channel_t channel, twr_dac_config_t *config);

//! @brief Start asynchronous DAC channel operation
//! @param[in] channel DAC channel
//! @return true On success
//! @return false On failure

bool twr_dac_async_run(twr_dac_channel_t channel);

//! @brief Stop asynchronous DAC channel operation
//! @param[in] channel DAC channel

void twr_dac_async_stop(twr_dac_channel_t channel);

//! @}

#endif // _TWR_DAC_H
