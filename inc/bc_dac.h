#ifndef _BC_DAC_H
#define _BC_DAC_H

#include <bc_common.h>

//! @addtogroup bc_dac bc_dac
//! @brief Driver for DAC (digital to analog converter)
//! @{

//! @brief DAC channel

typedef enum
{
    BC_DAC_CHANNEL_DAC0 = 0L, //!< DAC channel DAC0
    BC_DAC_CHANNEL_DAC1 = 1L, //!< DAC channel DAC1
    BC_DAC_CHANNEL_COUNT = 2L  //!< DAC channel count

} bc_dac_channel_t;

//! @brief DAC output format

typedef enum
{
    BC_DAC_FORMAT_8_BIT  = 0L, //!< DAC output format is 8-bit
    BC_DAC_FORMAT_16_BIT = 1L, //!< DAC output format is 16-bit
    BC_DAC_FORMAT_24_BIT = 2L, //!< DAC output format is 24-bit
    BC_DAC_FORMAT_32_BIT = 3L, //!< DAC output format is 32-bit

} bc_dac_format_t;

//! @brief Initialize DAC channel
//! @param[in] channel DAC channel
//! @param[in] format DAC output format

void bc_dac_init(bc_dac_channel_t channel, bc_dac_format_t format);

//! @brief Set DAC output format
//! @param[in] channel DAC channel
//! @param[in] format DAC output format

void bc_dac_set_format(bc_dac_channel_t channel, bc_dac_format_t format);

//! @brief Get DAC output format
//! @param[in] channel DAC channel
//! @return DAC output format

bc_dac_format_t bc_dac_get_format(bc_dac_channel_t channel);

//! @brief Perform measurement on DAC channel
//! @param[in] channel DAC channel
//! @param[out] output Pointer to source from which DAC output will be set

void bc_dac_set_output(bc_dac_channel_t channel, const void *output);

//! @}

#endif // _BC_DAC_H
