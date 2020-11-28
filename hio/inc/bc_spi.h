#ifndef _HIO_SPI_H
#define _HIO_SPI_H

#include <hio_common.h>

//! @addtogroup hio_spi hio_spi
//! @brief Driver for SPI bus
//! @{

//! @brief SPI communication speed

typedef enum
{
    //! @brief SPI communication speed is 125 kHz
    HIO_SPI_SPEED_125_KHZ = 0,

    //! @brief SPI communication speed is 250 kHz
    HIO_SPI_SPEED_250_KHZ = 1,

    //! @brief SPI communication speed is 500 kHz
    HIO_SPI_SPEED_500_KHZ = 2,

    //! @brief SPI communication speed is 1 MHz
    HIO_SPI_SPEED_1_MHZ = 3,

    //! @brief SPI communication speed is 2 MHz
    HIO_SPI_SPEED_2_MHZ = 4,

    //! @brief SPI communication speed is 4 MHz
    HIO_SPI_SPEED_4_MHZ = 5,

    //! @brief SPI communication speed is 8 MHz
    HIO_SPI_SPEED_8_MHZ = 6,

    //! @brief SPI communication speed is 16 MHz
    HIO_SPI_SPEED_16_MHZ = 7

} hio_spi_speed_t;

//! @brief SPI mode of operation

typedef enum
{
    //! @brief SPI mode of operation is 0 (CPOL = 0, CPHA = 0)
    HIO_SPI_MODE_0 = 0,

    //! @brief SPI mode of operation is 1 (CPOL = 0, CPHA = 1)
    HIO_SPI_MODE_1 = 1,

    //! @brief SPI mode of operation is 2 (CPOL = 1, CPHA = 0)
    HIO_SPI_MODE_2 = 2,

    //! @brief SPI mode of operation is 3 (CPOL = 1, CPHA = 1)
    HIO_SPI_MODE_3 = 3

} hio_spi_mode_t;

//! @brief SPI event

typedef enum
{
    //! @brief SPI event is completed
    HIO_SPI_EVENT_DONE = 1

} hio_spi_event_t;

//! @brief Initialize SPI channel
//! @param[in] speed SPI communication speed
//! @param[in] mode SPI mode of operation

void hio_spi_init(hio_spi_speed_t speed, hio_spi_mode_t mode);

//! @brief Set SPI communication speed
//! @param[in] speed SPI communication speed

void hio_spi_set_speed(hio_spi_speed_t speed);

//! @brief Set SPI timing
//! @param[in] cs_delay in us
//! @param[in] delay in us
//! @param[in] cs_quit in us

void hio_spi_set_timing(uint16_t cs_delay, uint16_t delay, uint16_t cs_quit);

//! @brief Get SPI communication speed
//! @return SPI communication speed

hio_spi_speed_t hio_spi_get_speed(void);

//! @brief Set SPI mode of operation
//! @param[in] mode SPI mode of operation

void hio_spi_set_mode(hio_spi_mode_t mode);

//! @brief Enable manual control of CS pin
//! @param[in] manual_cs_control enable manual control

void hio_spi_set_manual_cs_control(bool manual_cs_control);

//! @brief Get SPI mode of operation
//! @return SPI mode of operation

hio_spi_mode_t hio_spi_get_mode(void);

//! @brief Check if is ready for transfer
//! @return true If ready
//! @return false If not ready

bool hio_spi_is_ready(void);

//! @brief Execute SPI transfer
//! @param[in] source Pointer to source buffer
//! @param[out] destination Pointer to destination buffer
//! @param[in] length Number of bytes to be transferred

bool hio_spi_transfer(const void *source, void *destination, size_t length);

//! @brief Execute async SPI transfer
//! @param[in] source Pointer to source buffer
//! @param[out] destination Pointer to destination buffer
//! @param[in] length Number of bytes to be transferred
//! @param[in] event_handler Function address (can be NULL)
//! @param[in] event_param Optional event parameter (can be NULL)

bool hio_spi_async_transfer(const void *source, void *destination, size_t length, void (*event_handler)(hio_spi_event_t event, void *event_param), void (*event_param));

//! @}

#endif // _HIO_SPI_H
