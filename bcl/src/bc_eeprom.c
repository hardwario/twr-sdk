#include <bc_eeprom.h>
#include <bc_irq.h>
#include <stm32l0xx.h>
#include <bc_tick.h>
#include <bc_timer.h>
#include <bc_scheduler.h>

#define _BC_EEPROM_BASE DATA_EEPROM_BASE
#define _BC_EEPROM_END  DATA_EEPROM_BANK2_END
#define _BC_EEPROM_IS_BUSY() ((FLASH->SR & FLASH_SR_BSY) != 0UL)

static struct
{
    bool running;
    uint32_t address;
    uint8_t *buffer;
    size_t length;
    void (*event_handler)(bc_eepromc_event_t, void *);
    void *event_param;
    size_t i;
    bc_scheduler_task_id_t task_id;

} _bc_eeprom;

static bool _bc_eeprom_is_busy(bc_tick_t timeout);
static void _bc_eeprom_unlock(void);
static void _bc_eeprom_lock(void);
static bool _bc_eeprom_write(uint32_t address, size_t *i, uint8_t *buffer, size_t length);
static void _bc_eeprom_async_write_task(void *param);

bool bc_eeprom_write(uint32_t address, const void *buffer, size_t length)
{
    // Add EEPROM base offset to address
    address += _BC_EEPROM_BASE;

    // If user attempts to write outside EEPROM area...
    if ((address + length) > (_BC_EEPROM_END + 1))
    {
        // Indicate failure
        return false;
    }

    if (_bc_eeprom_is_busy(50))
    {
        return false;
    }

    _bc_eeprom_unlock();

    size_t i = 0;

    while (i < length)
    {
        _bc_eeprom_write(address, &i, (uint8_t *) buffer, length);
    }

    _bc_eeprom_lock();

    // If we do not read what we wrote...
    if (memcmp(buffer, (void *) address, length) != 0UL)
    {
        // Indicate failure
        return false;
    }

    // Indicate success
    return true;
}

bool bc_eeprom_async_write(uint32_t address, const void *buffer, size_t length, void (*event_handler)(bc_eepromc_event_t, void *), void *event_param)
{
    if (_bc_eeprom.running)
    {
        return false;
    }

    _bc_eeprom.address = address += _BC_EEPROM_BASE;

    // If user attempts to write outside EEPROM area...
    if ((_bc_eeprom.address + length) > (_BC_EEPROM_END + 1))
    {
        // Indicate failure
        return false;
    }

    _bc_eeprom.buffer = (uint8_t *) buffer;

    _bc_eeprom.length = length;

    _bc_eeprom.event_handler = event_handler;

    _bc_eeprom.event_param = event_param;

    _bc_eeprom.i = 0;

    _bc_eeprom.task_id = bc_scheduler_register(_bc_eeprom_async_write_task, NULL, 0);

    _bc_eeprom.running = true;

    return true;
}

void bc_eeprom_async_cancel(void)
{
    if (_bc_eeprom.running)
    {
        bc_scheduler_unregister(_bc_eeprom.task_id);

        _bc_eeprom.running = false;
    }
}

bool bc_eeprom_read(uint32_t address, void *buffer, size_t length)
{
    // Add EEPROM base offset to address
    address += _BC_EEPROM_BASE;

    // If user attempts to read outside of EEPROM boundary...
    if ((address + length) > (_BC_EEPROM_END + 1))
    {
        // Indicate failure
        return false;
    }

    // Read from EEPROM memory to buffer
    memcpy(buffer, (void *) address, length);

    // Indicate success
    return true;
}

size_t bc_eeprom_get_size(void)
{
    // Return EEPROM memory size
    return _BC_EEPROM_END - _BC_EEPROM_BASE + 1;
}

static bool _bc_eeprom_is_busy(bc_tick_t timeout)
{
    timeout += bc_tick_get();

    while (_BC_EEPROM_IS_BUSY())
    {
        if (timeout > bc_tick_get())
        {
            return true;
        }
    }

    return false;
}

static void _bc_eeprom_unlock(void)
{
    bc_irq_disable();

    // Unlock FLASH_PECR register
    if ((FLASH->PECR & FLASH_PECR_PELOCK) != 0)
    {
        FLASH->PEKEYR = FLASH_PEKEY1;
        FLASH->PEKEYR = FLASH_PEKEY2;
    }

    bc_irq_enable();
}

static void _bc_eeprom_lock(void)
{
    bc_irq_disable();

    // Lock FLASH_PECR register
    FLASH->PECR |= FLASH_PECR_PELOCK;

    bc_irq_enable();
}

static bool _bc_eeprom_write(uint32_t address, size_t *i, uint8_t *buffer, size_t length)
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

    while (_BC_EEPROM_IS_BUSY())
    {
        continue;
    }

    return write;
}


static void _bc_eeprom_async_write_task(void *param)
{
    (void) param;

    if (_BC_EEPROM_IS_BUSY())
    {
        bc_scheduler_plan_current_now();

        return;
    }

    _bc_eeprom_unlock();

    while(_bc_eeprom.i < _bc_eeprom.length)
    {
        if(_bc_eeprom_write(_bc_eeprom.address, &_bc_eeprom.i, _bc_eeprom.buffer, _bc_eeprom.length))
        {
            break;
        }
    };

    _bc_eeprom_lock();

    if(_bc_eeprom.i < _bc_eeprom.length)
    {
        bc_scheduler_plan_current_now();

        return;
    }

    _bc_eeprom.running = false;

    bc_scheduler_unregister(_bc_eeprom.task_id);

    if (memcmp(_bc_eeprom.buffer, (void *) _bc_eeprom.address, _bc_eeprom.length) != 0UL)
    {
        if (_bc_eeprom.event_handler != NULL)
        {
            _bc_eeprom.event_handler(BC_EEPROM_EVENT_ASYNC_WRITE_ERROR, _bc_eeprom.event_param);
        }
    }
    else
    {
        if (_bc_eeprom.event_handler != NULL)
        {
            _bc_eeprom.event_handler(BC_EEPROM_EVENT_ASYNC_WRITE_DONE, _bc_eeprom.event_param);
        }
    }
}
