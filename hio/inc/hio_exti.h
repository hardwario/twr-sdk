#ifndef _HIO_EXTI_H
#define _HIO_EXTI_H

#include <hio_common.h>

//! @addtogroup hio_exti hio_exti
//! @brief Driver for EXTI (external interrupts)
//! @{

//! @cond

#define _HIO_EXTI_PORT_A 0x00
#define _HIO_EXTI_PORT_B 0x10
#define _HIO_EXTI_PORT_C 0x20
#define _HIO_EXTI_PORT_H 0x50

//! @endcond

//! @brief EXTI lines

typedef enum
{
    //! @brief EXTI line PA0
    HIO_EXTI_LINE_PA0 = _HIO_EXTI_PORT_A | 0,

    //! @brief EXTI line PA1
    HIO_EXTI_LINE_PA1 = _HIO_EXTI_PORT_A | 1,

    //! @brief EXTI line PA2
    HIO_EXTI_LINE_PA2 = _HIO_EXTI_PORT_A | 2,

    //! @brief EXTI line PA3
    HIO_EXTI_LINE_PA3 = _HIO_EXTI_PORT_A | 3,

    //! @brief EXTI line PA4
    HIO_EXTI_LINE_PA4 = _HIO_EXTI_PORT_A | 4,

    //! @brief EXTI line PA5
    HIO_EXTI_LINE_PA5 = _HIO_EXTI_PORT_A | 5,

    //! @brief EXTI line PA6
    HIO_EXTI_LINE_PA6 = _HIO_EXTI_PORT_A | 6,

    //! @brief EXTI line PA7
    HIO_EXTI_LINE_PA7 = _HIO_EXTI_PORT_A | 7,

    //! @brief EXTI line PA8
    HIO_EXTI_LINE_PA8 = _HIO_EXTI_PORT_A | 8,

    //! @brief EXTI line PA9
    HIO_EXTI_LINE_PA9 = _HIO_EXTI_PORT_A | 9,

    //! @brief EXTI line PA10
    HIO_EXTI_LINE_PA10 = _HIO_EXTI_PORT_A | 10,

    //! @brief EXTI line PA11
    HIO_EXTI_LINE_PA11 = _HIO_EXTI_PORT_A | 11,

    //! @brief EXTI line PA12
    HIO_EXTI_LINE_PA12 = _HIO_EXTI_PORT_A | 12,

    //! @brief EXTI line PA13
    HIO_EXTI_LINE_PA13 = _HIO_EXTI_PORT_A | 13,

    //! @brief EXTI line PA14
    HIO_EXTI_LINE_PA14 = _HIO_EXTI_PORT_A | 14,

    //! @brief EXTI line PA15
    HIO_EXTI_LINE_PA15 = _HIO_EXTI_PORT_A | 15,

    //! @brief EXTI line PB0
    HIO_EXTI_LINE_PB0 = _HIO_EXTI_PORT_B | 0,

    //! @brief EXTI line PB1
    HIO_EXTI_LINE_PB1 = _HIO_EXTI_PORT_B | 1,

    //! @brief EXTI line PB2
    HIO_EXTI_LINE_PB2 = _HIO_EXTI_PORT_B | 2,

    //! @brief EXTI line PB3
    HIO_EXTI_LINE_PB3 = _HIO_EXTI_PORT_B | 3,

    //! @brief EXTI line PB4
    HIO_EXTI_LINE_PB4 = _HIO_EXTI_PORT_B | 4,

    //! @brief EXTI line PB5
    HIO_EXTI_LINE_PB5 = _HIO_EXTI_PORT_B | 5,

    //! @brief EXTI line PB6
    HIO_EXTI_LINE_PB6 = _HIO_EXTI_PORT_B | 6,

    //! @brief EXTI line PB7
    HIO_EXTI_LINE_PB7 = _HIO_EXTI_PORT_B | 7,

    //! @brief EXTI line PB8
    HIO_EXTI_LINE_PB8 = _HIO_EXTI_PORT_B | 8,

    //! @brief EXTI line PB9
    HIO_EXTI_LINE_PB9 = _HIO_EXTI_PORT_B | 9,

    //! @brief EXTI line PB10
    HIO_EXTI_LINE_PB10 = _HIO_EXTI_PORT_B | 10,

    //! @brief EXTI line PB11
    HIO_EXTI_LINE_PB11 = _HIO_EXTI_PORT_B | 11,

    //! @brief EXTI line PB12
    HIO_EXTI_LINE_PB12 = _HIO_EXTI_PORT_B | 12,

    //! @brief EXTI line PB13
    HIO_EXTI_LINE_PB13 = _HIO_EXTI_PORT_B | 13,

    //! @brief EXTI line PB14
    HIO_EXTI_LINE_PB14 = _HIO_EXTI_PORT_B | 14,

    //! @brief EXTI line PB15
    HIO_EXTI_LINE_PB15 = _HIO_EXTI_PORT_B | 15,

    //! @brief EXTI line PC13
    HIO_EXTI_LINE_PC13 = _HIO_EXTI_PORT_C | 13,

    //! @brief EXTI line PC14
    HIO_EXTI_LINE_PC14 = _HIO_EXTI_PORT_C | 14,

    //! @brief EXTI line PC15
    HIO_EXTI_LINE_PC15 = _HIO_EXTI_PORT_C | 15,

    //! @brief EXTI line PH0
    HIO_EXTI_LINE_PH0 = _HIO_EXTI_PORT_H | 0,

    //! @brief EXTI line PH1
    HIO_EXTI_LINE_PH1 = _HIO_EXTI_PORT_H | 1,

    //! @brief EXTI line P0
    HIO_EXTI_LINE_P0 = HIO_EXTI_LINE_PA0,

    //! @brief EXTI line P1
    HIO_EXTI_LINE_P1 = HIO_EXTI_LINE_PA1,

    //! @brief EXTI line P2
    HIO_EXTI_LINE_P2 = HIO_EXTI_LINE_PA2,

    //! @brief EXTI line P3
    HIO_EXTI_LINE_P3 = HIO_EXTI_LINE_PA3,

    //! @brief EXTI line P4
    HIO_EXTI_LINE_P4 = HIO_EXTI_LINE_PA4,

    //! @brief EXTI line P5
    HIO_EXTI_LINE_P5 = HIO_EXTI_LINE_PA5,

    //! @brief EXTI line P6
    HIO_EXTI_LINE_P6 = HIO_EXTI_LINE_PB1,

    //! @brief EXTI line P7
    HIO_EXTI_LINE_P7 = HIO_EXTI_LINE_PA6,

    //! @brief EXTI line P8
    HIO_EXTI_LINE_P8 = HIO_EXTI_LINE_PB0,

    //! @brief EXTI line P9
    HIO_EXTI_LINE_P9 = HIO_EXTI_LINE_PB2,

    //! @brief EXTI line P10
    HIO_EXTI_LINE_P10 = HIO_EXTI_LINE_PA10,

    //! @brief EXTI line P11
    HIO_EXTI_LINE_P11 = HIO_EXTI_LINE_PA9,

    //! @brief EXTI line P12
    HIO_EXTI_LINE_P12 = HIO_EXTI_LINE_PB14,

    //! @brief EXTI line P13
    HIO_EXTI_LINE_P13 = HIO_EXTI_LINE_PB15,

    //! @brief EXTI line P14
    HIO_EXTI_LINE_P14 = HIO_EXTI_LINE_PB13,

    //! @brief EXTI line P15
    HIO_EXTI_LINE_P15 = HIO_EXTI_LINE_PB12,

    //! @brief EXTI line P16
    HIO_EXTI_LINE_P16 = HIO_EXTI_LINE_PB8,

    //! @brief EXTI line P17
    HIO_EXTI_LINE_P17 = HIO_EXTI_LINE_PB9,

    //! @brief EXTI line BUTTON
    HIO_EXTI_LINE_BUTTON = HIO_EXTI_LINE_PA8

} hio_exti_line_t;

//! @brief Interrupt edge sensitivity

typedef enum
{
    //! @brief EXTI line is configured to rising edge sensitivity
    HIO_EXTI_EDGE_RISING = 0,

    //! @brief EXTI line is configured to falling edge sensitivity
    HIO_EXTI_EDGE_FALLING = 1,

    //! @brief EXTI line is configured to both rising and falling edge sensitivity
    HIO_EXTI_EDGE_RISING_AND_FALLING = 2

} hio_exti_edge_t;

//! @brief Enable EXTI line interrupt and register callback function
//! @param[in] line EXTI line
//! @param[in] edge Desired interrupt edge sensitivity
//! @param[in] callback Function address (called when interrupt occurs)
//! @param[in] param Optional parameter being passed to callback function (can be NULL)

void hio_exti_register(hio_exti_line_t line, hio_exti_edge_t edge, void (*callback)(hio_exti_line_t, void *), void *param);

//! @brief Disable EXTI line interrupt
//! @param[in] line EXTI line

void hio_exti_unregister(hio_exti_line_t line);

//! @}

#endif // _HIO_EXTI_H
