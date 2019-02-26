#ifndef _BC_DATA_STREAM_H
#define _BC_DATA_STREAM_H

#include <bc_common.h>

//! @addtogroup bc_data_stream bc_data_stream
//! @brief Library for computations on stream of data
//! @{

//! @brief Macro for float data stream buffer declaration

#define BC_DATA_STREAM_FLOAT_BUFFER(NAME, NUMBER_OF_SAMPLES) \
    float NAME##_feed[NUMBER_OF_SAMPLES]; \
    float NAME##_sort[NUMBER_OF_SAMPLES]; \
    bc_data_stream_buffer_t NAME = { \
            .feed = NAME##_feed, \
            .sort = NAME##_sort, \
            .number_of_samples = NUMBER_OF_SAMPLES, \
            .type=BC_DATA_STREAM_TYPE_FLOAT \
    };

//! @brief Macro for int data stream buffer declaration

#define BC_DATA_STREAM_INT_BUFFER(NAME, NUMBER_OF_SAMPLES) \
    int NAME##_feed[NUMBER_OF_SAMPLES]; \
    int NAME##_sort[NUMBER_OF_SAMPLES]; \
    bc_data_stream_buffer_t NAME = { \
            .feed = NAME##_feed, \
            .sort = NAME##_sort, \
            .number_of_samples = NUMBER_OF_SAMPLES, \
            .type=BC_DATA_STREAM_TYPE_INT \
    };

//! @brief Data stream type

typedef enum
{
    BC_DATA_STREAM_TYPE_FLOAT = 0,
    BC_DATA_STREAM_TYPE_INT = 1

} bc_data_stream_type_t;

//! @brief Buffer for data stream

typedef struct
{
    void *feed;
    void *sort;
    int number_of_samples;
    bc_data_stream_type_t type;

} bc_data_stream_buffer_t;


//! @brief Data stream instance

typedef struct bc_data_stream_t bc_data_stream_t;

//! @cond

struct bc_data_stream_t
{
    bc_data_stream_buffer_t *_buffer;
    int _counter;
    int _min_number_of_samples;
    int _feed_head;
};

//! @endcond

//! @brief Initialize data stream instance
//! @param[in] self Instance
//! @param[in] int min_number_of_samples minimal number of samples for calculation avarage, median ...
//! @param[in] buffer Buffer holding data stream content
//! @param[in] buffer_size Size of buffer holding data stream content

void bc_data_stream_init(bc_data_stream_t *self, int min_number_of_samples, bc_data_stream_buffer_t *buffer);

//! @brief Feed data into stream instance
//! @param[in] self Instance
//! @param[in] data Input data to be fed into data stream

void bc_data_stream_feed(bc_data_stream_t *self, void *data);

//! @brief Reset data stream
//! @param[in] self Instance

void bc_data_stream_reset(bc_data_stream_t *self);

//! @brief Get counter

int bc_data_stream_get_counter(bc_data_stream_t *self);

//! @brief Get length

int bc_data_stream_get_length(bc_data_stream_t *self);

//! @brief Get type

bc_data_stream_type_t bc_data_stream_get_type(bc_data_stream_t *self);

//! @brief Get buffer number_of_samples

int bc_data_stream_get_number_of_samples(bc_data_stream_t *self);

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

//! @brief Get nth value in data stream
//! @param[in] self Instance
//! @param[in] n position (example: 0 is first, -1 is last)
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool bc_data_stream_get_nth(bc_data_stream_t *self, int n, void *result);

//! @brief Get max value
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool bc_data_stream_get_max(bc_data_stream_t *self, void *result);

//! @brief Get min value
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool bc_data_stream_get_min(bc_data_stream_t *self, void *result);

//! @}

#endif // _BC_DATA_STREAM_H
