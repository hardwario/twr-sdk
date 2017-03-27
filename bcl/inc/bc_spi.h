#ifndef _BC_SPI_H
#define _BC_SPI_H

#include <bc_common.h>

//! @addtogroup bc_spi bc_spi
//! @brief Driver for SPI bus
//! @{

//! @brief SPI communication speed

typedef enum
{
    BC_SPI_SPEED_1_MHZ = 0, //!< SPI communication speed is 1 MHz
    BC_SPI_SPEED_2_MHZ = 1, //!< SPI communication speed is 2 MHz
    BC_SPI_SPEED_4_MHZ = 2, //!< SPI communication speed is 4 MHz
    BC_SPI_SPEED_8_MHZ = 3, //!< SPI communication speed is 8 MHz
    BC_SPI_SPEED_16_MHZ = 4 //!< SPI communication speed is 16 MHz

} bc_spi_speed_t;

//! @brief SPI mode of operation

typedef enum
{
    BC_SPI_MODE_0 = 0, //!< SPI mode of operation is 0 (CPOL = 0, CPHA = 0)
    BC_SPI_MODE_1 = 1, //!< SPI mode of operation is 1 (CPOL = 0, CPHA = 1)
    BC_SPI_MODE_2 = 2, //!< SPI mode of operation is 2 (CPOL = 1, CPHA = 0)
    BC_SPI_MODE_3 = 3  //!< SPI mode of operation is 3 (CPOL = 1, CPHA = 1)

} bc_spi_mode_t;

//! @brief Initialize SPI channel
//! @param[in] speed SPI communication speed
//! @param[in] mode SPI mode of operation

void bc_spi_init(bc_spi_speed_t speed, bc_spi_mode_t mode);

//! @brief Set SPI communication speed
//! @param[in] speed SPI communication speed

void bc_spi_set_speed(bc_spi_speed_t speed);

//! @brief Get SPI communication speed
//! @return SPI communication speed

bc_spi_speed_t bc_spi_get_speed(void);

//! @brief Set SPI mode of operation
//! @param[in] mode SPI mode of operation

void bc_spi_set_mode(bc_spi_mode_t mode);

//! @brief Get SPI mode of operation
//! @return SPI mode of operation

bc_spi_mode_t bc_spi_get_mode(void);

//! @brief Execute SPI transfer
//! @param[in] source Pointer to source buffer
//! @param[out] destination Pointer to destination buffer
//! @param[in] length Number of bytes to be transferred

void bc_spi_transfer(const void *source, void *destination, size_t length);

//! @}

#endif // _BC_SPI_H
