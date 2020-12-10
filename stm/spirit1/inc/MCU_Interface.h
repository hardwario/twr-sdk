/**
 * @file    MCU_Interface.h
 * @author  AMG - RF Application team
 * @version 3.2.4
 * @date    26-September-2016
 * @brief   Interface for the low level SPIRIT SPI driver.
 * @details
 * This header file constitutes an interface to the SPI driver used to
 * communicate with Spirit.
 * It exports some function prototypes to write/read registers and FIFOs
 * and to send command strobes.
 * Since the Spirit libraries are totally platform independent, the implementation
 * of these functions are not provided here. The user have to implement these functions
 * taking care to keep the exported prototypes.
 *
 * These functions are:
 *
 * <ul>
 * <li>SdkEvalSpiDeinit</li>
 * <li>SpiritSpiInit</li>
 * <li>SpiritSpiWriteRegisters</li>
 * <li>SpiritSpiReadRegisters</li>
 * <li>SpiritSpiCommandStrobes</li>
 * <li>SpiritSpiWriteLinearFifo</li>
 * <li>SpiritSpiReadLinearFifo</li>
 * </ul>
 *
 * @note An example of SPI driver implementation is available in the <i>Sdk_Eval</i> library.
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * THIS SOURCE CODE IS PROTECTED BY A LICENSE.
 * FOR MORE INFORMATION PLEASE CAREFULLY READ THE LICENSE AGREEMENT FILE LOCATED
 * IN THE ROOT DIRECTORY OF THIS FIRMWARE PACKAGE.
 *
 * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MCU_INTERFACE_H
#define __MCU_INTERFACE_H


/* Includes ------------------------------------------------------------------*/
#include "SPIRIT_Types.h"


#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup SPIRIT_Libraries
 * @{
 */


/** @defgroup SPIRIT_SPI_Driver         SPI Driver
 * @brief Header file for low level SPIRIT SPI driver.
 * @details See the file <i>@ref MCU_Interface.h</i> for more details.
 * @{
 */



/** @defgroup SPI_Exported_Types        SPI Exported Types
 * @{
 */

/**
 * @}
 */



/** @defgroup SPI_Exported_Constants    SPI Exported Constants
 * @{
 */

/**
 * @}
 */



/** @defgroup SPI_Exported_Macros       SPI Exported Macros
 * @{
 */

/**
 * @}
 */



/** @defgroup SPI_Exported_Functions    SPI Exported Functions
 * @{
 */

typedef SpiritStatus StatusBytes;

void SdkEvalSpiDeinit(void);
void SdkEvalSpiInit(void);
StatusBytes SdkEvalSpiWriteRegisters(uint8_t cRegAddress, uint8_t cNbBytes, uint8_t* pcBuffer);
StatusBytes SdkEvalSpiReadRegisters(uint8_t cRegAddress, uint8_t cNbBytes, uint8_t* pcBuffer);
StatusBytes SdkEvalSpiCommandStrobes(uint8_t cCommandCode);
StatusBytes SdkEvalSpiWriteFifo(uint8_t cNbBytes, uint8_t* pcBuffer);
StatusBytes SdkEvalSpiReadFifo(uint8_t cNbBytes, uint8_t* pcBuffer);

#define twr_spirit_status_t SpiritStatus

twr_spirit_status_t twr_spirit1_command(uint8_t command);
twr_spirit_status_t twr_spirit1_write(uint8_t address, const void *buffer, size_t length);
twr_spirit_status_t twr_spirit1_read(uint8_t address, void *buffer, size_t length);
void twr_spirit1_hal_init(void);
void twr_spirit1_hal_shutdown_low(void);
void twr_spirit1_hal_shutdown_high(void);

#define SpiritEnterShutdown                                            twr_spirit1_hal_shutdown_high
#define SpiritExitShutdown                                             twr_spirit1_hal_shutdown_low
#define SpiritSpiInit                                                  twr_spirit1_hal_init
#define SpiritSpiWriteRegisters(cRegAddress, cNbBytes, pcBuffer)       twr_spirit1_write(cRegAddress, pcBuffer, cNbBytes)
#define SpiritSpiReadRegisters(cRegAddress, cNbBytes, pcBuffer)        twr_spirit1_read(cRegAddress, pcBuffer, cNbBytes)
#define SpiritSpiCommandStrobes(cCommandCode)                          twr_spirit1_command(cCommandCode)
#define SpiritSpiWriteLinearFifo(cNbBytes, pcBuffer)                   twr_spirit1_write(0xFF, pcBuffer, cNbBytes)
#define SpiritSpiReadLinearFifo(cNbBytes, pcBuffer)                    twr_spirit1_read(0xFF, pcBuffer, cNbBytes)

/**
 * @}
 */

/**
 * @}
 */


/**
 * @}
 */



#ifdef __cplusplus
}
#endif

#endif

/******************* (C) COPYRIGHT 2016 STMicroelectronics *****END OF FILE****/
