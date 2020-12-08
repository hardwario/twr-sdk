#ifndef _TWR_SPI_H
#define _TWR_SPI_H

#include <twr_common.h>

//! @addtogroup twr_spi twr_spi
//! @brief Driver for SPI bus
//! @{

//! @brief SPI communication speed

typedef enum
{
    //! @brief SPI communication speed is 125 kHz
    TWR_SPI_SPEED_125_KHZ = 0,

    //! @brief SPI communication speed is 250 kHz
    TWR_SPI_SPEED_250_KHZ = 1,

    //! @brief SPI communication speed is 500 kHz
    TWR_SPI_SPEED_500_KHZ = 2,

    //! @brief SPI communication speed is 1 MHz
    TWR_SPI_SPEED_1_MHZ = 3,

    //! @brief SPI communication speed is 2 MHz
    TWR_SPI_SPEED_2_MHZ = 4,

    //! @brief SPI communication speed is 4 MHz
    TWR_SPI_SPEED_4_MHZ = 5,

    //! @brief SPI communication speed is 8 MHz
    TWR_SPI_SPEED_8_MHZ = 6,

    //! @brief SPI communication speed is 16 MHz
    TWR_SPI_SPEED_16_MHZ = 7

} twr_spi_speed_t;

//! @brief SPI mode of operation

typedef enum
{
    //! @brief SPI mode of operation is 0 (CPOL = 0, CPHA = 0)
    TWR_SPI_MODE_0 = 0,

    //! @brief SPI mode of operation is 1 (CPOL = 0, CPHA = 1)
    TWR_SPI_MODE_1 = 1,

    //! @brief SPI mode of operation is 2 (CPOL = 1, CPHA = 0)
    TWR_SPI_MODE_2 = 2,

    //! @brief SPI mode of operation is 3 (CPOL = 1, CPHA = 1)
    TWR_SPI_MODE_3 = 3

} twr_spi_mode_t;

//! @brief SPI event

typedef enum
{
    //! @brief SPI event is completed
    TWR_SPI_EVENT_DONE = 1

} twr_spi_event_t;

//! @brief Initialize SPI channel
//! @param[in] speed SPI communication speed
//! @param[in] mode SPI mode of operation

void twr_spi_init(twr_spi_speed_t speed, twr_spi_mode_t mode);

//! @brief Set SPI communication speed
//! @param[in] speed SPI communication speed

void twr_spi_set_speed(twr_spi_speed_t speed);

//! @brief Set SPI timing
//! @param[in] cs_delay in us
//! @param[in] delay in us
//! @param[in] cs_quit in us

void twr_spi_set_timing(uint16_t cs_delay, uint16_t delay, uint16_t cs_quit);

//! @brief Get SPI communication speed
//! @return SPI communication speed

twr_spi_speed_t twr_spi_get_speed(void);

//! @brief Set SPI mode of operation
//! @param[in] mode SPI mode of operation

void twr_spi_set_mode(twr_spi_mode_t mode);

//! @brief Enable manual control of CS pin
//! @param[in] manual_cs_control enable manual control

void twr_spi_set_manual_cs_control(bool manual_cs_control);

//! @brief Get SPI mode of operation
//! @return SPI mode of operation

twr_spi_mode_t twr_spi_get_mode(void);

//! @brief Check if is ready for transfer
//! @return true If ready
//! @return false If not ready

bool twr_spi_is_ready(void);

//! @brief Execute SPI transfer
//! @param[in] source Pointer to source buffer
//! @param[out] destination Pointer to destination buffer
//! @param[in] length Number of bytes to be transferred

bool twr_spi_transfer(const void *source, void *destination, size_t length);

//! @brief Execute async SPI transfer
//! @param[in] source Pointer to source buffer
//! @param[out] destination Pointer to destination buffer
//! @param[in] length Number of bytes to be transferred
//! @param[in] event_handler Function address (can be NULL)
//! @param[in] event_param Optional event parameter (can be NULL)

bool twr_spi_async_transfer(const void *source, void *destination, size_t length, void (*event_handler)(twr_spi_event_t event, void *event_param), void (*event_param));

//! @}

#endif // _TWR_SPI_H
