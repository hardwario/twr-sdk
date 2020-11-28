#ifndef _HIO_ONEWIRE_H
#define _HIO_ONEWIRE_H

#include <hio_gpio.h>

//! @addtogroup hio_onewire hio_onewire
//! @brief Driver for 1-Wire
//! @{

#define HIO_ONEWIRE_DEVICE_NUMBER_SKIP_ROM 0

//! @brief 1-Wire instance

typedef struct hio_onewire_t hio_onewire_t;

//! @cond

typedef struct
{
    bool (*init)(void *ctx);
    bool (*enable)(void *ctx);
    bool (*disable)(void *ctx);
    bool (*reset)(void *ctx);
    void (*write_bit)(void *ctx, uint8_t bit);
    uint8_t (*read_bit)(void *ctx);
    void (*write_byte)(void *ctx, uint8_t byte);
    uint8_t (*read_byte)(void *ctx);
    bool (*search_next)(void *ctx, hio_onewire_t *onewire, uint64_t *device_number);

} hio_onewire_driver_t;

struct hio_onewire_t
{
    uint8_t _last_discrepancy;
    uint8_t _last_family_discrepancy;
    bool _last_device_flag;
    uint8_t _last_rom_no[8];

    int _lock_count;
    void *_driver_ctx;
    const hio_onewire_driver_t *_driver;
    bool _auto_ds28e17_sleep_mode;
};

//! @endcond



void hio_onewire_init_gpio(hio_onewire_t *self, hio_gpio_channel_t channel);

//! @brief Initialize 1-Wire
//! @param[in] self Instance
//! @param[in] driver Driver
//! @param[in] driver_ctx Driver context

bool hio_onewire_init(hio_onewire_t *self, const hio_onewire_driver_t *driver, void *driver_ctx);

//! @brief Start transaction, enable pll and run timer
//! @param[in] self Instance
//! @return true On success
//! @return false On failure

bool hio_onewire_transaction_start(hio_onewire_t *self);

//! @brief Stop transaction
//! @param[in] self Instance
//! @return true On success
//! @return false On failure

bool hio_onewire_transaction_stop(hio_onewire_t *self);

//! @brief Is transaction run
//! @param[in] self Instance
//! @return true Transaction is run
//! @return false Transaction is stop

bool hio_onewire_is_transaction(hio_onewire_t *self);

//! @brief Reset the 1-Wire bus and return the presence of any device
//! @param[in] self Instance
//! @return true Device present
//! @return false No device present

bool hio_onewire_reset(hio_onewire_t *self);

//! @brief Select device
//! @param[in] self Instance
//! @param[in] device_number Device number (for 0 skip ROM)

void hio_onewire_select(hio_onewire_t *self, uint64_t *device_number);

//! @brief Skip ROM
//! @param[in] self Instance

void hio_onewire_skip_rom(hio_onewire_t *self);

//! @brief Select device
//! @param[in] self Instance
//! @param[in] data Input data to be written
//! @param[in] length Number of bytes to be written

void hio_onewire_write(hio_onewire_t *self, const void *buffer, size_t length);

//! @brief Select device
//! @param[in] self Instance
//! @param[out] data Output which have been read
//! @param[in] length Number of bytes to be read

void hio_onewire_read(hio_onewire_t *self, void *buffer, size_t length);

//! @brief Select device
//! @param[in] self Instance
//! @param[in] data Input data to be written

void hio_onewire_write_byte(hio_onewire_t *self, uint8_t data);

//! @brief Select device
//! @param[in] self Instance
//! @return data which have been read

uint8_t hio_onewire_read_byte(hio_onewire_t *self);

//! @brief Select device
//! @param[in] self Instance
//! @param[in] bit Input bit to be written

void hio_onewire_write_bit(hio_onewire_t *self, int bit);

//! @brief Select device
//! @param[in] self Instance
//! @return bit which have been read

int hio_onewire_read_bit(hio_onewire_t *self);

//! @brief Search for all devices on 1-Wire
//! @param[in] channel GPIO channel
//! @param[out] device_list Pointer to destination array holding list of devices
//! @param[in] device_list_size Size of array holding list of devices
//! @return Number of found devices

int hio_onewire_search_all(hio_onewire_t *self, uint64_t *device_list, size_t device_list_size);

//! @brief Search for all devices on 1-Wire with family code
//! @param[in] channel GPIO channel
//! @param[in] family_code
//! @param[out] device_list Pointer to destination array holding list of devices
//! @param[in] device_list_size Size of array holding list of devices
//! @return Number of found devices

int hio_onewire_search_family(hio_onewire_t *self, uint8_t family_code, uint64_t *device_list, size_t device_list_size);

//! @brief Start of manual search, see also hio_onewire_search_next
//! @param[in] channel GPIO channel
//! @param[in] family_code Family code of 1-Wire device or NULL

void hio_onewire_search_start(hio_onewire_t *self, uint8_t family_code);

//! @brief Manual search of next device
//! @param[in] device_number GPIO channel
//! @param[in] device_number 64b device number
//! @return true if new device was found, false if ther are no more devices on the bus

bool hio_onewire_search_next(hio_onewire_t *self, uint64_t *device_number);

//! @brief Enable call sleep mode for all ds28e17 after transaction
//! @param[in] on

void hio_onewire_auto_ds28e17_sleep_mode(hio_onewire_t *self, bool on);

//! @brief Calculate 8-bit CRC
//! @param[in] buffer
//! @param[in] length Number of bytes
//! @param[in] The crc starting value
//! @return Calculated CRC

uint8_t hio_onewire_crc8(const void *buffer, size_t length, uint8_t crc);

//! @brief Calculate 16-bit CRC, polynomial 0x8005
//! @param[in] buffer
//! @param[in] length Number of bytes
//! @param[in] The crc starting value
//! @return Calculated CRC

uint16_t hio_onewire_crc16(const void *buffer, size_t length, uint16_t crc);

//! @}

#endif // _HIO_ONEWIRE_H
