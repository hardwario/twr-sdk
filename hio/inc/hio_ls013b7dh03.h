#ifndef _HIO_LS013B7DH03_H
#define _HIO_LS013B7DH03_H

#include <hio_gfx.h>
#include <hio_scheduler.h>

//! @addtogroup hio_ls013b7dh03 hio_ls013b7dh03
//! @brief Driver for LS013B7DH03 1.28" HR-TFT Memory LCD
//! @{

// http://www.mouser.com/ds/2/365/LS013B7DH03%20SPEC_SMA-224806.pdf
// https://www.embeddedartists.com/sites/default/files/support/datasheet/Memory_LCD_Programming.pdf
// See app note https://www.silabs.com/documents/public/application-notes/AN0048.pdf
// Figure 3.1
// 1B mode | 1B addr + 16B data + 1B dummy | 1B dummy END
#define HIO_LS013B7DH03_FRAMEBUFFER_SIZE (1 + ((1+16+1) * 128) + 1)
#define HIO_LS013B7DH03_WIDTH 128
#define HIO_LS013B7DH03_HEIGHT 128

//! @brief Instance

typedef struct
{
    uint8_t _framebuffer[HIO_LS013B7DH03_FRAMEBUFFER_SIZE];
    uint8_t _vcom;
    hio_scheduler_task_id_t _task_id;
    bool (*_pin_cs_set)(bool state);

} hio_ls013b7dh03_t;

//! @brief Initialize lcd driver
//! @param[in] self Instance

void hio_ls013b7dh03_init(hio_ls013b7dh03_t *self, bool (*pin_cs_set)(bool state));

//! @brief Get capabilities
//! @param[in] self Instance

hio_gfx_caps_t hio_ls013b7dh03_get_caps(hio_ls013b7dh03_t *self);

//! @brief Check if lcd is ready for commands
//! @param[in] self Instance
//! @return true If ready
//! @return false If not ready

bool hio_ls013b7dh03_is_ready(hio_ls013b7dh03_t *self);

//! @brief Clear
//! @param[in] self Instance

void hio_ls013b7dh03_clear(hio_ls013b7dh03_t *self);

//! @brief Lcd draw pixel
//! @param[in] self Instance
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] color Pixels state

void hio_ls013b7dh03_draw_pixel(hio_ls013b7dh03_t *self, int x, int y, uint32_t color);

//! @brief Lcd get pixel
//! @param[in] self Instance
//! @param[in] left Pixels from left edge
//! @param[in] top Pixels from top edge
//! @param[in] color Pixels state

uint32_t hio_ls013b7dh03_get_pixel(hio_ls013b7dh03_t *self, int x, int y);

//! @brief Lcd update, send data
//! @param[in] self Instance
//! @return true On success
//! @return false On failure

bool hio_ls013b7dh03_update(hio_ls013b7dh03_t *self);

//! @brief Send Lcd clear memory command
//! @return true On success
//! @return false On failure

bool hio_ls013b7dh03_clear_memory_command(hio_ls013b7dh03_t *self);

//! @brief Get Lcd driver

const hio_gfx_driver_t *hio_ls013b7dh03_get_driver(void);

//! @}

#endif //_HIO_LS013B7DH03_H
