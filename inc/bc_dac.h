#ifndef _BC_DAC_H
#define _BC_DAC_H

#include <bc_common.h>

//! @addtogroup bc_dac bc_dac
//! @brief Driver for DAC (digital to analog converter)
//! @{

//! @brief DAC channel

typedef enum
{
    //! @brief DAC channel DAC0
    BC_DAC_DAC0 = 0,

    //! @brief DAC channel DAC1
    BC_DAC_DAC1 = 1

} bc_dac_channel_t;

//! @brief Raw value format

typedef enum
{
    //! @brief Raw value format is 8-bit
    BC_DAC_FORMAT_8_BIT  = 0,

    //! @brief Raw value format is 16-bit
    BC_DAC_FORMAT_16_BIT = 1,

    //! @brief Raw value format is 24-bit
    BC_DAC_FORMAT_24_BIT = 2,

    //! @brief Raw value format is 32-bit
    BC_DAC_FORMAT_32_BIT = 3

} bc_dac_format_t;

//! @brief Initialize DAC channel
//! @param[in] channel DAC channel
//! @param[in] format DAC raw value format

void bc_dac_init(bc_dac_channel_t channel, bc_dac_format_t format);

//! @brief Set raw value format
//! @param[in] channel DAC channel
//! @param[in] format Desired raw value format

void bc_dac_set_format(bc_dac_channel_t channel, bc_dac_format_t format);

//! @brief Get raw value format
//! @param[in] channel DAC channel
//! @return Raw value format

bc_dac_format_t bc_dac_get_format(bc_dac_channel_t channel);

//! @brief Set DAC channel output as raw value
//! @param[in] channel DAC channel
//! @param[out] raw Pointer to desired raw value

void bc_dac_set_output_raw(bc_dac_channel_t channel, const void *raw);

//! @brief Set DAC channel output as voltage
//! @param[in] channel DAC channel
//! @param[out] voltage Desired voltage

void bc_dac_set_output_voltage(bc_dac_channel_t channel, float voltage);

//! @}

#endif // _BC_DAC_H
