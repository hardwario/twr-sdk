#ifndef _TWR_DATA_STREAM_H
#define _TWR_DATA_STREAM_H

#include <twr_common.h>

//! @addtogroup twr_data_stream twr_data_stream
//! @brief Library for computations on stream of data
//! @{

//! @brief Macro for float data stream buffer declaration

#define TWR_DATA_STREAM_FLOAT_BUFFER(NAME, NUMBER_OF_SAMPLES) \
    float NAME##_feed[NUMBER_OF_SAMPLES]; \
    float NAME##_sort[NUMBER_OF_SAMPLES]; \
    twr_data_stream_buffer_t NAME = { \
            .feed = NAME##_feed, \
            .sort = NAME##_sort, \
            .number_of_samples = NUMBER_OF_SAMPLES, \
            .type=TWR_DATA_STREAM_TYPE_FLOAT \
    };

//! @brief Macro for int data stream buffer declaration

#define TWR_DATA_STREAM_INT_BUFFER(NAME, NUMBER_OF_SAMPLES) \
    int NAME##_feed[NUMBER_OF_SAMPLES]; \
    int NAME##_sort[NUMBER_OF_SAMPLES]; \
    twr_data_stream_buffer_t NAME = { \
            .feed = NAME##_feed, \
            .sort = NAME##_sort, \
            .number_of_samples = NUMBER_OF_SAMPLES, \
            .type=TWR_DATA_STREAM_TYPE_INT \
    };

//! @brief Macro for float data stream array declaration

#define TWR_DATA_STREAM_FLOAT_ARRAY(NAME, COUNT, NUMBER_OF_SAMPLES) \
    static float NAME##_feed[(COUNT)][(NUMBER_OF_SAMPLES)]; \
    static float NAME##_sort[(NUMBER_OF_SAMPLES)]; \
    static twr_data_stream_buffer_t NAME##_buffer[(COUNT)]; \
    static twr_data_stream_t NAME[(COUNT)];

//! @brief Macro for float data stream array initialization

#define TWR_DATA_STREAM_FLOAT_ARRAY_INIT(NAME, COUNT, MIN_NUMBER_OF_SAMPLES) \
    for (size_t i = 0; i < (COUNT); i++) \
    { \
        NAME##_buffer[i].feed = NAME##_feed[i]; \
        NAME##_buffer[i].sort = NAME##_sort; \
        NAME##_buffer[i].number_of_samples = (sizeof(NAME##_feed[i]) / sizeof(float)); \
        NAME##_buffer[i].type=TWR_DATA_STREAM_TYPE_FLOAT; \
        twr_data_stream_init(&NAME[i], (MIN_NUMBER_OF_SAMPLES), &NAME##_buffer[i]); \
    }

//! @brief Data stream type

typedef enum
{
    TWR_DATA_STREAM_TYPE_FLOAT = 0,
    TWR_DATA_STREAM_TYPE_INT = 1

} twr_data_stream_type_t;

//! @brief Buffer for data stream

typedef struct
{
    void *feed;
    void *sort;
    int number_of_samples;
    twr_data_stream_type_t type;

} twr_data_stream_buffer_t;


//! @brief Data stream instance

typedef struct twr_data_stream_t twr_data_stream_t;

//! @cond

struct twr_data_stream_t
{
    twr_data_stream_buffer_t *_buffer;
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

void twr_data_stream_init(twr_data_stream_t *self, int min_number_of_samples, twr_data_stream_buffer_t *buffer);

//! @brief Feed data into stream instance
//! @param[in] self Instance
//! @param[in] data Input data to be fed into data stream

void twr_data_stream_feed(twr_data_stream_t *self, void *data);

//! @brief Reset data stream
//! @param[in] self Instance

void twr_data_stream_reset(twr_data_stream_t *self);

//! @brief Get counter

int twr_data_stream_get_counter(twr_data_stream_t *self);

//! @brief Get length

int twr_data_stream_get_length(twr_data_stream_t *self);

//! @brief Get type

twr_data_stream_type_t twr_data_stream_get_type(twr_data_stream_t *self);

//! @brief Get buffer number_of_samples

int twr_data_stream_get_number_of_samples(twr_data_stream_t *self);

//! @brief Get average value of data stream
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool twr_data_stream_get_average(twr_data_stream_t *self, void *result);

//! @brief Get median value of data stream
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool twr_data_stream_get_median(twr_data_stream_t *self, void *result);

//! @brief Get first value in data stream
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool twr_data_stream_get_first(twr_data_stream_t *self, void *result);

//! @brief Get last value in data stream
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool twr_data_stream_get_last(twr_data_stream_t *self, void *result);

//! @brief Get nth value in data stream
//! @param[in] self Instance
//! @param[in] n position (example: 0 is first, -1 is last)
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool twr_data_stream_get_nth(twr_data_stream_t *self, int n, void *result);

//! @brief Get max value
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool twr_data_stream_get_max(twr_data_stream_t *self, void *result);

//! @brief Get min value
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool twr_data_stream_get_min(twr_data_stream_t *self, void *result);

//! @}

#endif // _TWR_DATA_STREAM_H
