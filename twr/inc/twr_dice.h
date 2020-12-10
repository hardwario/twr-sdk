#ifndef _TWR_DICE_H
#define _TWR_DICE_H

#include <twr_common.h>

//! @addtogroup twr_dice twr_dice
//! @brief Helper library to determine dice (cube) face position from vectors
//! @{

//! @brief Dice faces

typedef enum
{
    //! @brief Unknown dice face
    TWR_DICE_FACE_UNKNOWN = 0,

    //! @brief Dice face 1
    TWR_DICE_FACE_1 = 1,

    //! @brief Dice face 2
    TWR_DICE_FACE_2 = 2,

    //! @brief Dice face 3
    TWR_DICE_FACE_3 = 3,

    //! @brief Dice face 4
    TWR_DICE_FACE_4 = 4,

    //! @brief Dice face 5
    TWR_DICE_FACE_5 = 5,

    //! @brief Dice face 6
    TWR_DICE_FACE_6 = 6

} twr_dice_face_t;

//! @brief Dice instance

typedef struct twr_dice_t twr_dice_t;

//! @cond

struct twr_dice_t
{
    twr_dice_face_t _face;
    float _threshold;
};

//! @endcond

//! @brief Initialize dice
//! @param[in] self Instance
//! @param[in] start Dice initial face

void twr_dice_init(twr_dice_t *self, twr_dice_face_t start);

//! @brief Set threshold
//! @param[in] self Instance
//! @param[in] threshold

void twr_dice_set_threshold(twr_dice_t *self, float threshold);

//! @brief Feed dice with X/Y/Z axis vectors
//! @param[in] self Instance
//! @param[in] x_axis Vector of X axis
//! @param[in] y_axis Vector of Y axis
//! @param[in] z_axis Vector of Z axis

void twr_dice_feed_vectors(twr_dice_t *self, float x_axis, float y_axis, float z_axis);

//! @brief Get calculated dice face
//! @param[in] self Instance
//! @return Dice face

twr_dice_face_t twr_dice_get_face(twr_dice_t *self);

//! @}

#endif // _TWR_DICE_H
