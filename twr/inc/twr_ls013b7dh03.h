#ifndef _TWR_LS013B7DH03_H
#define _TWR_LS013B7DH03_H

#include <twr_gfx.h>
#include <twr_scheduler.h>

//! @addtogroup twr_ls013b7dh03 twr_ls013b7dh03
//! @brief Driver for LS013B7DH03 1.28" HR-TFT Memory LCD
//! @{

// http://www.mouser.com/ds/2/365/LS013B7DH03%20SPEC_SMA-224806.pdf
// https://www.embeddedartists.com/sites/default/files/support/datasheet/Memory_LCD_Programming.pdf
// See app note https://www.silabs.com/documents/public/application-notes/AN0048.pdf
// Figure 3.1

#if TWR_LS013B7DH03_400x200 == 1
    // For SHARP LS027B7DH01A bigger display
    #define TWR_LS013B7DH03_WIDTH 400
    #define TWR_LS013B7DH03_HEIGHT 240
#else
    #define TWR_LS013B7DH03_WIDTH 128
    #define TWR_LS013B7DH03_HEIGHT 128
#endif

// 1B mode | 1B addr + 16B data + 1B dummy | 1B dummy END
#define TWR_LS013B7DH03_FRAMEBUFFER_SIZE (1 + ((1+(TWR_LS013B7DH03_WIDTH / 8)+1) * TWR_LS013B7DH03_HEIGHT) + 1)


//! @brief Instance

typedef struct
{
    uint8_t _framebuffer[TWR_LS013B7DH03_FRAMEBUFFER_SIZE];
    uint8_t _vcom;
    twr_scheduler_task_id_t _task_id;
    bool (*_pin_cs_set)(bool state);

} twr_ls013b7dh03_t;

//! @brief Initialize lcd driver
//! @param[in] self Instance

void twr_ls013b7dh03_init(twr_ls013b7dh03_t *self, bool (*pin_cs_set)(bool state));

//! @brief Get capabilities
//! @param[in] self Instance

twr_gfx_caps_t twr_ls013b7dh03_get_caps(twr_ls013b7dh03_t *self);

//! @brief Check if lcd is ready for commands
//! @param[in] self Instance
//! @return true If ready
//! @return false If not ready

bool twr_ls013b7dh03_is_ready(twr_ls013b7dh03_t *self);

//! @brief Clear
//! @param[in] self Instance

void twr_ls013b7dh03_clear(twr_ls013b7dh03_t *self);

//! @brief Lcd draw pixel
//! @param[in] self Instance
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] color Pixels state

void twr_ls013b7dh03_draw_pixel(twr_ls013b7dh03_t *self, int x, int y, uint32_t color);

//! @brief Lcd get pixel
//! @param[in] self Instance
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] color Pixels state

uint32_t twr_ls013b7dh03_get_pixel(twr_ls013b7dh03_t *self, int x, int y);

//! @brief Lcd update, send data
//! @param[in] self Instance
//! @return true On success
//! @return false On failure

bool twr_ls013b7dh03_update(twr_ls013b7dh03_t *self);

//! @brief Send Lcd clear memory command
//! @return true On success
//! @return false On failure

bool twr_ls013b7dh03_clear_memory_command(twr_ls013b7dh03_t *self);

//! @brief Get Lcd driver

const twr_gfx_driver_t *twr_ls013b7dh03_get_driver(void);

//! @}

#endif //_TWR_LS013B7DH03_H
