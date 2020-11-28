#ifndef _HIO_DATA_STREAM_H
#define _HIO_DATA_STREAM_H

#include <hio_common.h>

//! @addtogroup hio_data_stream hio_data_stream
//! @brief Library for computations on stream of data
//! @{

//! @brief Macro for float data stream buffer declaration

#define HIO_DATA_STREAM_FLOAT_BUFFER(NAME, NUMBER_OF_SAMPLES) \
    float NAME##_feed[NUMBER_OF_SAMPLES]; \
    float NAME##_sort[NUMBER_OF_SAMPLES]; \
    hio_data_stream_buffer_t NAME = { \
            .feed = NAME##_feed, \
            .sort = NAME##_sort, \
            .number_of_samples = NUMBER_OF_SAMPLES, \
            .type=HIO_DATA_STREAM_TYPE_FLOAT \
    };

//! @brief Macro for int data stream buffer declaration

#define HIO_DATA_STREAM_INT_BUFFER(NAME, NUMBER_OF_SAMPLES) \
    int NAME##_feed[NUMBER_OF_SAMPLES]; \
    int NAME##_sort[NUMBER_OF_SAMPLES]; \
    hio_data_stream_buffer_t NAME = { \
            .feed = NAME##_feed, \
            .sort = NAME##_sort, \
            .number_of_samples = NUMBER_OF_SAMPLES, \
            .type=HIO_DATA_STREAM_TYPE_INT \
    };

//! @brief Macro for float data stream array declaration

#define HIO_DATA_STREAM_FLOAT_ARRAY(NAME, COUNT, NUMBER_OF_SAMPLES) \
    static float NAME##_feed[(COUNT)][(NUMBER_OF_SAMPLES)]; \
    static float NAME##_sort[(NUMBER_OF_SAMPLES)]; \
    static hio_data_stream_buffer_t NAME##_buffer[(COUNT)]; \
    static hio_data_stream_t NAME[(COUNT)];

//! @brief Macro for float data stream array initialization

#define HIO_DATA_STREAM_FLOAT_ARRAY_INIT(NAME, COUNT, MIN_NUMBER_OF_SAMPLES) \
    for (size_t i = 0; i < (COUNT); i++) \
    { \
        NAME##_buffer[i].feed = NAME##_feed[i]; \
        NAME##_buffer[i].sort = NAME##_sort; \
        NAME##_buffer[i].number_of_samples = (sizeof(NAME##_feed[i]) / sizeof(float)); \
        NAME##_buffer[i].type=HIO_DATA_STREAM_TYPE_FLOAT; \
        hio_data_stream_init(&NAME[i], (MIN_NUMBER_OF_SAMPLES), &NAME##_buffer[i]); \
    }

//! @brief Data stream type

typedef enum
{
    HIO_DATA_STREAM_TYPE_FLOAT = 0,
    HIO_DATA_STREAM_TYPE_INT = 1

} hio_data_stream_type_t;

//! @brief Buffer for data stream

typedef struct
{
    void *feed;
    void *sort;
    int number_of_samples;
    hio_data_stream_type_t type;

} hio_data_stream_buffer_t;


//! @brief Data stream instance

typedef struct hio_data_stream_t hio_data_stream_t;

//! @cond

struct hio_data_stream_t
{
    hio_data_stream_buffer_t *_buffer;
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

void hio_data_stream_init(hio_data_stream_t *self, int min_number_of_samples, hio_data_stream_buffer_t *buffer);

//! @brief Feed data into stream instance
//! @param[in] self Instance
//! @param[in] data Input data to be fed into data stream

void hio_data_stream_feed(hio_data_stream_t *self, void *data);

//! @brief Reset data stream
//! @param[in] self Instance

void hio_data_stream_reset(hio_data_stream_t *self);

//! @brief Get counter

int hio_data_stream_get_counter(hio_data_stream_t *self);

//! @brief Get length

int hio_data_stream_get_length(hio_data_stream_t *self);

//! @brief Get type

hio_data_stream_type_t hio_data_stream_get_type(hio_data_stream_t *self);

//! @brief Get buffer number_of_samples

int hio_data_stream_get_number_of_samples(hio_data_stream_t *self);

//! @brief Get average value of data stream
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool hio_data_stream_get_average(hio_data_stream_t *self, void *result);

//! @brief Get median value of data stream
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool hio_data_stream_get_median(hio_data_stream_t *self, void *result);

//! @brief Get first value in data stream
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool hio_data_stream_get_first(hio_data_stream_t *self, void *result);

//! @brief Get last value in data stream
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool hio_data_stream_get_last(hio_data_stream_t *self, void *result);

//! @brief Get nth value in data stream
//! @param[in] self Instance
//! @param[in] n position (example: 0 is first, -1 is last)
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool hio_data_stream_get_nth(hio_data_stream_t *self, int n, void *result);

//! @brief Get max value
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool hio_data_stream_get_max(hio_data_stream_t *self, void *result);

//! @brief Get min value
//! @param[in] self Instance
//! @param[out] self Pointer to buffer where result will be stored
//! @return true On success (desired value is available)
//! @return false On failure (desired value is not available)

bool hio_data_stream_get_min(hio_data_stream_t *self, void *result);

//! @}

#endif // _HIO_DATA_STREAM_H
