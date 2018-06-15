#ifndef _BC_DICE_H
#define _BC_DICE_H

#include <bc_common.h>

//! @addtogroup bc_dice bc_dice
//! @brief Helper library to determine dice (cube) face position from vectors
//! @{

//! @brief Dice faces

typedef enum
{
    //! @brief Unknown dice face
    BC_DICE_FACE_UNKNOWN = 0,

    //! @brief Dice face 1
    BC_DICE_FACE_1 = 1,

    //! @brief Dice face 2
    BC_DICE_FACE_2 = 2,

    //! @brief Dice face 3
    BC_DICE_FACE_3 = 3,

    //! @brief Dice face 4
    BC_DICE_FACE_4 = 4,

    //! @brief Dice face 5
    BC_DICE_FACE_5 = 5,

    //! @brief Dice face 6
    BC_DICE_FACE_6 = 6

} bc_dice_face_t;

//! @brief Dice instance

typedef struct bc_dice_t bc_dice_t;

//! @cond

struct bc_dice_t
{
    bc_dice_face_t _face;
    float _threshold;
};

//! @endcond

//! @brief Initialize dice
//! @param[in] self Instance
//! @param[in] start Dice initial face

void bc_dice_init(bc_dice_t *self, bc_dice_face_t start);

//! @brief Set threshold
//! @param[in] self Instance
//! @param[in] threshold

void bc_dice_set_threshold(bc_dice_t *self, float threshold);

//! @brief Feed dice with X/Y/Z axis vectors
//! @param[in] self Instance
//! @param[in] x_axis Vector of X axis
//! @param[in] y_axis Vector of Y axis
//! @param[in] z_axis Vector of Z axis

void bc_dice_feed_vectors(bc_dice_t *self, float x_axis, float y_axis, float z_axis);

//! @brief Get calculated dice face
//! @param[in] self Instance
//! @return Dice face

bc_dice_face_t bc_dice_get_face(bc_dice_t *self);

//! @}

#endif // _BC_DICE_H
