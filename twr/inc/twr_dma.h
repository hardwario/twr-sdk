#ifndef _TWR_DMA_H
#define _TWR_DMA_H

#include <twr_common.h>

//! @addtogroup twr_dma twr_dma
//! @brief Driver for DMA
//! @{

//! @brief DMA channels

typedef enum
{
    //! @brief DMA channel 1
    TWR_DMA_CHANNEL_1 = 0,

    //! @brief DMA channel 2
    TWR_DMA_CHANNEL_2 = 1,

    //! @brief DMA channel 3
    TWR_DMA_CHANNEL_3 = 2,

    //! @brief DMA channel 4
    TWR_DMA_CHANNEL_4 = 3,

    //! @brief DMA channel 5, used for SPI
    TWR_DMA_CHANNEL_5 = 4,

    //! @brief DMA channel 6
    TWR_DMA_CHANNEL_6 = 5,

    //! @brief DMA channel 7
    TWR_DMA_CHANNEL_7 = 6

} twr_dma_channel_t;

//! @brief DMA requests

typedef enum
{
    //! @brief DMA request 0
    TWR_DMA_REQUEST_0 = 0,

    //! @brief DMA request 1
    TWR_DMA_REQUEST_1 = 1,

    //! @brief DMA request 2
    TWR_DMA_REQUEST_2 = 2,

    //! @brief DMA request 3
    TWR_DMA_REQUEST_3 = 3,

    //! @brief DMA request 4
    TWR_DMA_REQUEST_4 = 4,

    //! @brief DMA request 5
    TWR_DMA_REQUEST_5 = 5,

    //! @brief DMA request 6
    TWR_DMA_REQUEST_6 = 6,

    //! @brief DMA request 7
    TWR_DMA_REQUEST_7 = 7,

    //! @brief DMA request 8
    TWR_DMA_REQUEST_8 = 8,

    //! @brief DMA request 9
    TWR_DMA_REQUEST_9 = 9,

    //! @brief DMA request 10
    TWR_DMA_REQUEST_10 = 10,

    //! @brief DMA request 11
    TWR_DMA_REQUEST_11 = 11,

    //! @brief DMA request 12
    TWR_DMA_REQUEST_12 = 12,

    //! @brief DMA request 13
    TWR_DMA_REQUEST_13 = 13,

    //! @brief DMA request 14
    TWR_DMA_REQUEST_14 = 14,

    //! @brief DMA request 15
    TWR_DMA_REQUEST_15 = 15

} twr_dma_request_t;

//! @brief DMA channel directions

typedef enum
{
    //! @brief DMA channel direction from RAM to peripheral
    TWR_DMA_DIRECTION_TO_PERIPHERAL = 0,

    //! @brief DMA channel direction from peripheral to RAM
    TWR_DMA_DIRECTION_TO_RAM = 1

} twr_dma_direction_t;

//! @brief DMA channel data size

typedef enum
{
    //! @brief DMA channel data size 1B
    TWR_DMA_SIZE_1 = 0,

    //! @brief DMA channel data size 2B
    TWR_DMA_SIZE_2 = 1,

    //! @brief DMA channel data size 4B
    TWR_DMA_SIZE_4 = 2

} twr_dma_size_t;

//! @brief DMA channel mode

typedef enum
{
    //! @brief DMA channel mode standard
    TWR_DMA_MODE_STANDARD = 0,

    //! @brief DMA channel mode circular
    TWR_DMA_MODE_CIRCULAR = 1

} twr_dma_mode_t;

//! @brief DMA channel event

typedef enum
{
    //! @brief DMA channel event error
    TWR_DMA_EVENT_ERROR = 0,

    //! @brief DMA channel event half done
    TWR_DMA_EVENT_HALF_DONE = 1,

    //! @brief DMA channel event done
    TWR_DMA_EVENT_DONE = 2

} twr_dma_event_t;

//! @brief DMA channel priority

typedef enum
{
    //! @brief DMA channel priority is low
    TWR_DMA_PRIORITY_LOW = 0,

    //! @brief DMA channel priority is medium
    TWR_DMA_PRIORITY_MEDIUM = 1,

    //! @brief DMA channel priority is high
    TWR_DMA_PRIORITY_HIGH = 2,

    //! @brief DMA channel priority is very high
    TWR_DMA_PRIORITY_VERY_HIGH = 3

} twr_dma_priority_t;

//! @brief DMA channel configuration

typedef struct
{
    //! @brief DMA channel request
    twr_dma_request_t request;

    //! @brief DMA channel direction
    twr_dma_direction_t direction;

    //! @brief DMA channel memory data size
    twr_dma_size_t data_size_memory;

    //! @brief DMA channel peripheral data size
    twr_dma_size_t data_size_peripheral;

    //! @brief DMA channel data length
    size_t length;

    //! @brief DMA channel mode
    twr_dma_mode_t mode;

    //! @brief RAM memory address
    void *address_memory;

    //! @brief Peripheral address
    void *address_peripheral;

    //! @brief DMA channel priority
    twr_dma_priority_t priority;

} twr_dma_channel_config_t;

//! @brief Initialize DMA

void twr_dma_init(void);

//! @brief Configure DMA channel
//! @param[in] channel DMA channel
//! @param[in] config Pointer to DMA channel configuration

void twr_dma_channel_config(twr_dma_channel_t channel, twr_dma_channel_config_t *config);

//! @brief Set callback function
//! @param[in] channel DMA channel
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void twr_dma_set_event_handler(twr_dma_channel_t channel, void (*event_handler)(twr_dma_channel_t, twr_dma_event_t, void *), void *event_param);

//! @brief Start DMA channel
//! @param[in] channel DMA channel

void twr_dma_channel_run(twr_dma_channel_t channel);

//! @brief Stop DMA channel
//! @param[in] channel DMA channel

void twr_dma_channel_stop(twr_dma_channel_t channel);

size_t twr_dma_channel_get_length(twr_dma_channel_t channel);


//! @}

#endif // _TWR_DMA_H
