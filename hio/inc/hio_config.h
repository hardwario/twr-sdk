#ifndef _HIO_CONFIG_H
#define _HIO_CONFIG_H

#include <hio_common.h>

//! @addtogroup hio_config hio_config
//! @brief Library for saving and loading configuration to EEPROM
//! @{

//! @brief Initialize and load the config from EEPROM
//! @param[in] signature Any number specifying current configuration version.
//! @param[in] config Pointer to configuration structure
//! @param[in] size Size of the configuration structure
//! @param[in] init_config Pointer to default configoration or null

void hio_config_init(uint64_t signature, void *config, size_t size, void *init_config);

//! @brief Reset EEPROM configuration to zeros or init_config

void hio_config_reset(void);

//! @brief Load EEPROM configuration
//! @return true When configuration is valid
//! @return false When configuration is invalid

bool hio_config_load(void);

//! @brief Save configuration to EEPROM
//! @return true When configuration was saved
//! @return false When there is a write error

bool hio_config_save(void);

//! @}

#endif
