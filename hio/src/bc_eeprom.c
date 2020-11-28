#include <hio_eeprom.h>
#include <hio_irq.h>
#include <stm32l0xx.h>
#include <hio_tick.h>
#include <hio_timer.h>
#include <hio_scheduler.h>

#define _HIO_EEPROM_BASE DATA_EEPROM_BASE
#define _HIO_EEPROM_END  DATA_EEPROM_BANK2_END
#define _HIO_EEPROM_IS_BUSY() ((FLASH->SR & FLASH_SR_BSY) != 0UL)

static struct
{
    bool running;
    uint32_t address;
    uint8_t *buffer;
    size_t length;
    void (*event_handler)(hio_eepromc_event_t, void *);
    void *event_param;
    size_t i;
    hio_scheduler_task_id_t task_id;

} _hio_eeprom;

static bool _hio_eeprom_is_busy(hio_tick_t timeout);
static void _hio_eeprom_unlock(void);
static void _hio_eeprom_lock(void);
static bool _hio_eeprom_write(uint32_t address, size_t *i, uint8_t *buffer, size_t length);
static void _hio_eeprom_async_write_task(void *param);

bool hio_eeprom_write(uint32_t address, const void *buffer, size_t length)
{
    // Add EEPROM base offset to address
    address += _HIO_EEPROM_BASE;

    // If user attempts to write outside EEPROM area...
    if ((address + length) > (_HIO_EEPROM_END + 1))
    {
        // Indicate failure
        return false;
    }

    if (_hio_eeprom_is_busy(50))
    {
        return false;
    }

    _hio_eeprom_unlock();

    size_t i = 0;

    while (i < length)
    {
        _hio_eeprom_write(address, &i, (uint8_t *) buffer, length);
    }

    _hio_eeprom_lock();

    // If we do not read what we wrote...
    if (memcmp(buffer, (void *) address, length) != 0UL)
    {
        // Indicate failure
        return false;
    }

    // Indicate success
    return true;
}

bool hio_eeprom_async_write(uint32_t address, const void *buffer, size_t length, void (*event_handler)(hio_eepromc_event_t, void *), void *event_param)
{
    if (_hio_eeprom.running)
    {
        return false;
    }

    _hio_eeprom.address = address += _HIO_EEPROM_BASE;

    // If user attempts to write outside EEPROM area...
    if ((_hio_eeprom.address + length) > (_HIO_EEPROM_END + 1))
    {
        // Indicate failure
        return false;
    }

    _hio_eeprom.buffer = (uint8_t *) buffer;

    _hio_eeprom.length = length;

    _hio_eeprom.event_handler = event_handler;

    _hio_eeprom.event_param = event_param;

    _hio_eeprom.i = 0;

    _hio_eeprom.task_id = hio_scheduler_register(_hio_eeprom_async_write_task, NULL, 0);

    _hio_eeprom.running = true;

    return true;
}

void hio_eeprom_async_cancel(void)
{
    if (_hio_eeprom.running)
    {
        hio_scheduler_unregister(_hio_eeprom.task_id);

        _hio_eeprom.running = false;
    }
}

bool hio_eeprom_read(uint32_t address, void *buffer, size_t length)
{
    // Add EEPROM base offset to address
    address += _HIO_EEPROM_BASE;

    // If user attempts to read outside of EEPROM boundary...
    if ((address + length) > (_HIO_EEPROM_END + 1))
    {
        // Indicate failure
        return false;
    }

    // Read from EEPROM memory to buffer
    memcpy(buffer, (void *) address, length);

    // Indicate success
    return true;
}

size_t hio_eeprom_get_size(void)
{
    // Return EEPROM memory size
    return _HIO_EEPROM_END - _HIO_EEPROM_BASE + 1;
}

static bool _hio_eeprom_is_busy(hio_tick_t timeout)
{
    timeout += hio_tick_get();

    while (_HIO_EEPROM_IS_BUSY())
    {
        if (timeout > hio_tick_get())
        {
            return true;
        }
    }

    return false;
}

static void _hio_eeprom_unlock(void)
{
    hio_irq_disable();

    // Unlock FLASH_PECR register
    if ((FLASH->PECR & FLASH_PECR_PELOCK) != 0)
    {
        FLASH->PEKEYR = FLASH_PEKEY1;
        FLASH->PEKEYR = FLASH_PEKEY2;
    }

    hio_irq_enable();
}

static void _hio_eeprom_lock(void)
{
    hio_irq_disable();

    // Lock FLASH_PECR register
    FLASH->PECR |= FLASH_PECR_PELOCK;

    hio_irq_enable();
}

static bool _hio_eeprom_write(uint32_t address, size_t *i, uint8_t *buffer, size_t length)
{
    uint32_t addr = address + *i;

    uint8_t mod = addr % 4;

    bool write = false;

    if (mod == 0)
    {
        if (*i + 4 > length)
        {
            mod = (addr % 2) + 2;
        }
    }

    if (mod == 2)
    {
        if (*i + 2 > length)
        {
            mod = 1;
        }
    }

    if (mod == 0)
    {
        uint32_t value = ((uint32_t) buffer[*i + 3]) << 24 | ((uint32_t) buffer[*i + 2]) << 16 | ((uint32_t) buffer[*i + 1]) << 8 | buffer[*i];

        if (*((uint32_t *) addr) != value)
        {
            *((uint32_t *) addr) = value;

            write = true;
        }

        *i += 4;
    }
    else if (mod == 2)
    {
        uint16_t value = ((uint16_t) buffer[*i + 1]) << 8 | (uint16_t) buffer[*i];

        if (*((uint16_t *) addr) != value)
        {
            *((uint16_t *) addr) = value;

            write = true;
        }

        *i += 2;
    }
    else
    {
        uint8_t value = buffer[*i];

        if (*((uint8_t *) addr) != value)
        {
            *((uint8_t *) addr) = value;

            write = true;
        }

        *i += 1;
    }

    while (_HIO_EEPROM_IS_BUSY())
    {
        continue;
    }

    return write;
}


static void _hio_eeprom_async_write_task(void *param)
{
    (void) param;

    if (_HIO_EEPROM_IS_BUSY())
    {
        hio_scheduler_plan_current_now();

        return;
    }

    _hio_eeprom_unlock();

    while(_hio_eeprom.i < _hio_eeprom.length)
    {
        if(_hio_eeprom_write(_hio_eeprom.address, &_hio_eeprom.i, _hio_eeprom.buffer, _hio_eeprom.length))
        {
            break;
        }
    };

    _hio_eeprom_lock();

    if(_hio_eeprom.i < _hio_eeprom.length)
    {
        hio_scheduler_plan_current_now();

        return;
    }

    _hio_eeprom.running = false;

    hio_scheduler_unregister(_hio_eeprom.task_id);

    if (memcmp(_hio_eeprom.buffer, (void *) _hio_eeprom.address, _hio_eeprom.length) != 0UL)
    {
        if (_hio_eeprom.event_handler != NULL)
        {
            _hio_eeprom.event_handler(HIO_EEPROM_EVENT_ASYNC_WRITE_ERROR, _hio_eeprom.event_param);
        }
    }
    else
    {
        if (_hio_eeprom.event_handler != NULL)
        {
            _hio_eeprom.event_handler(HIO_EEPROM_EVENT_ASYNC_WRITE_DONE, _hio_eeprom.event_param);
        }
    }
}
