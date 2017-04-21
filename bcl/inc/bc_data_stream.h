#ifndef _BC_DATA_STREAM_H
#define _BC_DATA_STREAM_H

#include <bc_common.h>

//! @addtogroup bc_data_stream bc_data_stream
//! @brief Library for computations on stream of data
//! @{

//! @brief Macro for float data stream buffer declaration

#define BC_DATA_STREAM_FLOAT(NAME, NUMBER_OF_SAMPLES) float (NAME)[(2 * NUMBER_OF_SAMPLES)];

//! @brief Data stream type

typedef enum
{
    BC_DATA_STREAM_TYPE_FLOAT = 0

} bc_data_stream_type_t;

//! @brief Data stream instance

typedef struct bc_data_stream_t bc_data_stream_t;

//! @cond

struct
{
    bc_data_stream_type_t _type;
    void *_buffer;
    size_t _buffer_size;
    void *_buffer_head;
    void *_buffer_tail;

} bc_data_stream_t;

//! @endcond

//! @brief Initialize data stream instance
//! @param[in] self Instance
//! @param[in] type Type of data stream values
//! @param[in] buffer Buffer holding data stream content
//! @param[in] buffer_size Size of buffer holding data stream content

void bc_data_stream_init(bc_data_stream_t *self, bc_data_stream_type_t type, void *buffer, size_t buffer_size);

//! @brief Initialize data stream instance
//! @param[in] self Instance
//! @param[in] data Input data to be fed into data stream

void bc_data_stream_feed(bc_data_stream_t *self, void *data);

//! @brief Reset data stream
//! @param[in] self Instance

void bc_data_stream_reset(bc_data_stream_t *self);

//! @brief Get average value of data stream
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool bc_data_stream_get_average(bc_data_stream_t *self, void *result);

//! @brief Get median value of data stream
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool bc_data_stream_get_median(bc_data_stream_t *self, void *result);

//! @brief Get first value in data stream
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool bc_data_stream_get_first(bc_data_stream_t *self, void *result);

//! @brief Get last value in data stream
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool bc_data_stream_get_last(bc_data_stream_t *self, void *result);

//! @}

#endif // _BC_DATA_STREAM_H
