#ifndef _BC_DMA_H
#define _BC_DMA_H

//! @addtogroup bc_dma bc_dma
//! @brief Driver for DMA
//! @{

//! @brief DMA channels

typedef enum
{
    //! @brief DMA channel 1
    BC_DMA_CHANNEL_1 = 0,

    //! @brief DMA channel 2
    BC_DMA_CHANNEL_2 = 1,

    //! @brief DMA channel 3
    BC_DMA_CHANNEL_3 = 2,

    //! @brief DMA channel 4
    BC_DMA_CHANNEL_4 = 3,

    //! @brief DMA channel 5
    BC_DMA_CHANNEL_5 = 4,

    //! @brief DMA channel 6
    BC_DMA_CHANNEL_6 = 5,

    //! @brief DMA channel 7
    BC_DMA_CHANNEL_7 = 6

} bc_dma_channel_t;

//! @brief DMA requests

typedef enum
{
    //! @brief DMA request 0
    BC_DMA_REQUEST_0 = 0,
    
    //! @brief DMA request 1
    BC_DMA_REQUEST_1 = 1,
    
    //! @brief DMA request 2
    BC_DMA_REQUEST_2 = 2,
    
    //! @brief DMA request 3
    BC_DMA_REQUEST_3 = 3,
    
    //! @brief DMA request 4
    BC_DMA_REQUEST_4 = 4,
    
    //! @brief DMA request 5
    BC_DMA_REQUEST_5 = 5,
    
    //! @brief DMA request 6
    BC_DMA_REQUEST_6 = 6,
    
    //! @brief DMA request 7
    BC_DMA_REQUEST_7 = 7,
    
    //! @brief DMA request 8
    BC_DMA_REQUEST_8 = 8,
    
    //! @brief DMA request 9
    BC_DMA_REQUEST_9 = 9,
    
    //! @brief DMA request 10
    BC_DMA_REQUEST_10 = 10,
    
    //! @brief DMA request 11
    BC_DMA_REQUEST_11 = 11,
    
    //! @brief DMA request 12
    BC_DMA_REQUEST_12 = 12,
    
    //! @brief DMA request 13
    BC_DMA_REQUEST_13 = 13,
    
    //! @brief DMA request 14
    BC_DMA_REQUEST_14 = 14,
    
    //! @brief DMA request 15
    BC_DMA_REQUEST_15 = 15

} bc_dma_request_t;

//! @brief DMA channel directions

typedef enum
{
    //! @brief DMA channel direction from RAM to peripheral
    BC_DMA_DIRECTION_TO_PERIPHERAL,
    
    //! @brief DMA channel direction from peripheral to RAM
    BC_DMA_DIRECTION_TO_RAM,
    
    //! @brief DMA channel direction from RAM to RAM
    BC_DMA_DIRECTION_M2M // TODO not implemented

} bc_dma_direction_t;

//! @brief DMA channel data size

typedef enum
{
    //! @brief DMA channel data size 1B
    BC_DMA_SIZE_1,
    
    //! @brief DMA channel data size 2B
    BC_DMA_SIZE_2,
    
    //! @brief DMA channel data size 4B
    BC_DMA_SIZE_4

} bc_dma_size_t;

//! @brief DMA channel mode

typedef enum
{
    //! @brief DMA channel mode standard
    BC_DMA_MODE_STANDARD,
    
    //! @brief DMA channel mode circular
    BC_DMA_MODE_CIRCULAR

} bc_dma_mode_t;

//! @brief DMA channel event

typedef enum
{    
    //! @brief DMA channel event error
    BC_DMA_EVENT_ERROR = 0,

    //! @brief DMA channel event half done
    BC_DMA_EVENT_HALF_DONE = 1,
    
    //! @brief DMA channel event done
    BC_DMA_EVENT_DONE = 2

} bc_dma_event_t;

//! @brief DMA channel priority

typedef enum
{
    //! @brief DMA channel priority is low
    BC_DMA_PRIORITY_LOW = 0,
    
    //! @brief DMA channel priority is medium
    BC_DMA_PRIORITY_MEDIUM = 1,
    
    //! @brief DMA channel priority is high
    BC_DMA_PRIORITY_HIGH = 2,
    
    //! @brief DMA channel priority is very high
    BC_DMA_PRIORITY_VERY_HIGH = 3

} bc_dma_priority_t;

//! @brief DMA channel configuration

typedef struct
{
    //! @brief DMA channel request
    bc_dma_request_t request;
    
    //! @brief DMA channel direction
    bc_dma_direction_t direction;
    
    //! @brief DMA channel memory data size
    bc_dma_size_t data_size_memory;

    //! @brief DMA channel peripheral data size
    bc_dma_size_t data_size_peripheral;
    
    //! @brief DMA channel data length
    uint32_t length;
    
    //! @brief DMA channel mode
    bc_dma_mode_t mode;
    
    //! @brief RAM memory address (TODO look at M2M)
    void *address_memory;
    
    //! @brief Peripheral address (TODO look at M2M)
    void *address_peripheral;
    
    //! @brief DMA channel priority
    bc_dma_priority_t priority;

} bc_dma_channel_config_t;

typedef void bc_dma_event_handler_t(bc_dma_channel_t, bc_dma_event_t, void *);

//! @brief Initialize DMA

void bc_dma_init(void);

//! @brief Configure DMA channel
//! @param[in] channel DMA channel
//! @param[in] config Pointer to DMA channel configuration

void bc_dma_channel_config(bc_dma_channel_t channel, bc_dma_channel_config_t *config);

//! @brief Set callback function
//! @param[in] channel DMA channel
//! @param[in] event_handler Function address
//! @param[in] event_param Optional event parameter (can be NULL)

void bc_dma_set_event_handler(bc_dma_channel_t channel, bc_dma_event_handler_t *event_handler, void *event_param);

//! @}

#endif // _BC_DMA_H
