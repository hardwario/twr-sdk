#ifndef _BC_DMA_H
#define _BC_DMA_H

typedef enum
{
    BC_DMA_CHANNEL_1 = 0,
    BC_DMA_CHANNEL_2 = 1,
    BC_DMA_CHANNEL_3 = 2,
    BC_DMA_CHANNEL_4 = 3,
    BC_DMA_CHANNEL_5 = 4,
    BC_DMA_CHANNEL_6 = 5,
    BC_DMA_CHANNEL_7 = 6

} bc_dma_channel_t;

typedef enum
{
    BC_DMA_REQUEST_0 = 0,
    BC_DMA_REQUEST_1 = 1,
    BC_DMA_REQUEST_2 = 2,
    BC_DMA_REQUEST_3 = 3,
    BC_DMA_REQUEST_4 = 4,
    BC_DMA_REQUEST_5 = 5,
    BC_DMA_REQUEST_6 = 6,
    BC_DMA_REQUEST_7 = 7,
    BC_DMA_REQUEST_8 = 8,
    BC_DMA_REQUEST_9 = 9,
    BC_DMA_REQUEST_10 = 10,
    BC_DMA_REQUEST_11 = 11,
    BC_DMA_REQUEST_12 = 12,
    BC_DMA_REQUEST_13 = 13,
    BC_DMA_REQUEST_14 = 14,
    BC_DMA_REQUEST_15 = 15

} bc_dma_request_t;

typedef enum
{
    BC_DMA_DIRECTION_TO_PERIPHERAL,
    BC_DMA_DIRECTION_TO_RAM,
    BC_DMA_DIRECTION_M2M // TODO not implemented

} bc_dma_direction_t;

typedef enum
{
    BC_DMA_SIZE_1,
    BC_DMA_SIZE_2,
    BC_DMA_SIZE_4

} bc_dma_size_t;

typedef enum
{
    BC_DMA_MODE_STANDARD,
    BC_DMA_MODE_CIRCULAR

} bc_dma_mode_t;

typedef enum
{
    BC_DMA_EVENT_ERROR = 0,
    BC_DMA_EVENT_HALF_DONE = 1,
    BC_DMA_EVENT_DONE = 2

} bc_dma_event_t;

typedef enum
{
    BC_DMA_PRIORITY_LOW = 0,
    BC_DMA_PRIORITY_MEDIUM = 1,
    BC_DMA_PRIORITY_HIGH = 2,
    BC_DMA_PRIORITY_VERY_HIGH = 3

} bc_dma_priority_t;

typedef struct
{
    bc_dma_request_t request;
    bc_dma_direction_t direction;
    bc_dma_size_t size;
    uint32_t length;
    bc_dma_mode_t mode;
    void *address_memory;
    void *address_peripheral;
    bc_dma_priority_t priority;

} bc_dma_channel_config_t;

typedef void bc_dma_event_handler_t(bc_dma_channel_t, bc_dma_event_t, void *);

void bc_dma_init(void);

void bc_dma_channel_config(bc_dma_channel_t channel, bc_dma_channel_config_t *config);

void bc_dma_set_event_handler(bc_dma_channel_t channel, bc_dma_event_handler_t *event_handler, void *event_param);

#endif // _BC_DMA_H
