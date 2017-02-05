#ifndef _BC_EXTI_H
#define _BC_EXTI_H

#include <bc_common.h>

//! @addtogroup bc_exti bc_exti
//! @brief Driver for EXTI (external interrupts)
//! @{

//! @cond

#define _BC_EXTI_PORT_A 0x00
#define _BC_EXTI_PORT_B 0x10
#define _BC_EXTI_PORT_C 0x20
#define _BC_EXTI_PORT_H 0x50

//! @endcond

//! @brief EXTI lines

typedef enum
{
    //! @brief EXTI line PA0
    BC_EXTI_LINE_PA0 = _BC_EXTI_PORT_A | 0,

    //! @brief EXTI line PA1
    BC_EXTI_LINE_PA1 = _BC_EXTI_PORT_A | 1,

    //! @brief EXTI line PA2
    BC_EXTI_LINE_PA2 = _BC_EXTI_PORT_A | 2,

    //! @brief EXTI line PA3
    BC_EXTI_LINE_PA3 = _BC_EXTI_PORT_A | 3,

    //! @brief EXTI line PA4
    BC_EXTI_LINE_PA4 = _BC_EXTI_PORT_A | 4,

    //! @brief EXTI line PA5
    BC_EXTI_LINE_PA5 = _BC_EXTI_PORT_A | 5,

    //! @brief EXTI line PA6
    BC_EXTI_LINE_PA6 = _BC_EXTI_PORT_A | 6,

    //! @brief EXTI line PA7
    BC_EXTI_LINE_PA7 = _BC_EXTI_PORT_A | 7,

    //! @brief EXTI line PA8
    BC_EXTI_LINE_PA8 = _BC_EXTI_PORT_A | 8,

    //! @brief EXTI line PA9
    BC_EXTI_LINE_PA9 = _BC_EXTI_PORT_A | 9,

    //! @brief EXTI line PA10
    BC_EXTI_LINE_PA10 = _BC_EXTI_PORT_A | 10,

    //! @brief EXTI line PA11
    BC_EXTI_LINE_PA11 = _BC_EXTI_PORT_A | 11,

    //! @brief EXTI line PA12
    BC_EXTI_LINE_PA12 = _BC_EXTI_PORT_A | 12,

    //! @brief EXTI line PA13
    BC_EXTI_LINE_PA13 = _BC_EXTI_PORT_A | 13,

    //! @brief EXTI line PA14
    BC_EXTI_LINE_PA14 = _BC_EXTI_PORT_A | 14,

    //! @brief EXTI line PA15
    BC_EXTI_LINE_PA15 = _BC_EXTI_PORT_A | 15,

    //! @brief EXTI line PB0
    BC_EXTI_LINE_PB0 = _BC_EXTI_PORT_B | 0,

    //! @brief EXTI line PB1
    BC_EXTI_LINE_PB1 = _BC_EXTI_PORT_B | 1,

    //! @brief EXTI line PB2
    BC_EXTI_LINE_PB2 = _BC_EXTI_PORT_B | 2,

    //! @brief EXTI line PB3
    BC_EXTI_LINE_PB3 = _BC_EXTI_PORT_B | 3,

    //! @brief EXTI line PB4
    BC_EXTI_LINE_PB4 = _BC_EXTI_PORT_B | 4,

    //! @brief EXTI line PB5
    BC_EXTI_LINE_PB5 = _BC_EXTI_PORT_B | 5,

    //! @brief EXTI line PB6
    BC_EXTI_LINE_PB6 = _BC_EXTI_PORT_B | 6,

    //! @brief EXTI line PB7
    BC_EXTI_LINE_PB7 = _BC_EXTI_PORT_B | 7,

    //! @brief EXTI line PB8
    BC_EXTI_LINE_PB8 = _BC_EXTI_PORT_B | 8,

    //! @brief EXTI line PB9
    BC_EXTI_LINE_PB9 = _BC_EXTI_PORT_B | 9,

    //! @brief EXTI line PB10
    BC_EXTI_LINE_PB10 = _BC_EXTI_PORT_B | 10,

    //! @brief EXTI line PB11
    BC_EXTI_LINE_PB11 = _BC_EXTI_PORT_B | 11,

    //! @brief EXTI line PB12
    BC_EXTI_LINE_PB12 = _BC_EXTI_PORT_B | 12,

    //! @brief EXTI line PB13
    BC_EXTI_LINE_PB13 = _BC_EXTI_PORT_B | 13,

    //! @brief EXTI line PB14
    BC_EXTI_LINE_PB14 = _BC_EXTI_PORT_B | 14,

    //! @brief EXTI line PB15
    BC_EXTI_LINE_PB15 = _BC_EXTI_PORT_B | 15,

    //! @brief EXTI line PC13
    BC_EXTI_LINE_PC13 = _BC_EXTI_PORT_C | 13,

    //! @brief EXTI line PC14
    BC_EXTI_LINE_PC14 = _BC_EXTI_PORT_C | 14,

    //! @brief EXTI line PC15
    BC_EXTI_LINE_PC15 = _BC_EXTI_PORT_C | 15,

    //! @brief EXTI line PH0
    BC_EXTI_LINE_PH0 = _BC_EXTI_PORT_H | 0,

    //! @brief EXTI line PH1
    BC_EXTI_LINE_PH1 = _BC_EXTI_PORT_H | 1

} bc_exti_line_t;

//! @brief Interrupt edge sensitivity

typedef enum
{
    //! @brief EXTI line is configured to rising edge sensitivity
    BC_EXTI_SENSITIVITY_RISING = 0,

    //! @brief EXTI line is configured to falling edge sensitivity
    BC_EXTI_SENSITIVITY_FALLING = 1,

    //! @brief EXTI line is configured to both rising and falling edge sensitivity
    BC_EXTI_SENSITIVITY_RISING_AND_FALLING = 2

} bc_exti_sensitivity_t;

//! @brief Enable EXTI line interrupt and register callback function
//! @param[in] line EXTI line
//! @param[in] sensitivity Desired interrupt edge sensitivity
//! @param[in] callback Function address (called when interrupt occurs)

void bc_exti_register(bc_exti_line_t line, bc_exti_sensitivity_t sensitivity, void (*callback)(bc_exti_line_t));

//! @brief Disable EXTI line interrupt
//! @param[in] line EXTI line

void bc_exti_unregister(bc_exti_line_t line);

//! @}

#endif // _BC_EXTI_H
