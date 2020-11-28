#ifndef _HIO_DICE_H
#define _HIO_DICE_H

#include <hio_common.h>

//! @addtogroup hio_dice hio_dice
//! @brief Helper library to determine dice (cube) face position from vectors
//! @{

//! @brief Dice faces

typedef enum
{
    //! @brief Unknown dice face
    HIO_DICE_FACE_UNKNOWN = 0,

    //! @brief Dice face 1
    HIO_DICE_FACE_1 = 1,

    //! @brief Dice face 2
    HIO_DICE_FACE_2 = 2,

    //! @brief Dice face 3
    HIO_DICE_FACE_3 = 3,

    //! @brief Dice face 4
    HIO_DICE_FACE_4 = 4,

    //! @brief Dice face 5
    HIO_DICE_FACE_5 = 5,

    //! @brief Dice face 6
    HIO_DICE_FACE_6 = 6

} hio_dice_face_t;

//! @brief Dice instance

typedef struct hio_dice_t hio_dice_t;

//! @cond

struct hio_dice_t
{
    hio_dice_face_t _face;
    float _threshold;
};

//! @endcond

//! @brief Initialize dice
//! @param[in] self Instance
//! @param[in] start Dice initial face

void hio_dice_init(hio_dice_t *self, hio_dice_face_t start);

//! @brief Set threshold
//! @param[in] self Instance
//! @param[in] threshold

void hio_dice_set_threshold(hio_dice_t *self, float threshold);

//! @brief Feed dice with X/Y/Z axis vectors
//! @param[in] self Instance
//! @param[in] x_axis Vector of X axis
//! @param[in] y_axis Vector of Y axis
//! @param[in] z_axis Vector of Z axis

void hio_dice_feed_vectors(hio_dice_t *self, float x_axis, float y_axis, float z_axis);

//! @brief Get calculated dice face
//! @param[in] self Instance
//! @return Dice face

hio_dice_face_t hio_dice_get_face(hio_dice_t *self);

//! @}

#endif // _HIO_DICE_H
