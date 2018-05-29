#include <bc_eeprom.h>
#include <bc_irq.h>
#include <stm32l0xx.h>
#include <bc_tick.h>
#include <bc_timer.h>

#define _BC_EEPROM_BASE DATA_EEPROM_BASE
#define _BC_EEPROM_END  DATA_EEPROM_BANK2_END

static bool _bc_eeprom_is_busy(bc_tick_t timeout);

bool bc_eeprom_write(uint32_t address, const void *buffer, size_t length)
{
    // Add EEPROM base offset to address
    uint32_t addr = address + _BC_EEPROM_BASE;

    // If user attempts to write outside EEPROM area...
    if ((addr + length) > (_BC_EEPROM_END + 1))
    {
        // Indicate failure
        return false;
    }

    if (_bc_eeprom_is_busy(50))
    {
        return false;
    }

    // Disable interrupts
    bc_irq_disable();

    // Unlock FLASH_PECR register
    if ((FLASH->PECR & FLASH_PECR_PELOCK) != 0)
    {
        FLASH->PEKEYR = FLASH_PEKEY1;
        FLASH->PEKEYR = FLASH_PEKEY2;
    }

    // Enable interrupts
    bc_irq_enable();

    size_t i = 0;
    uint8_t mod;

    uint8_t *ptr = (uint8_t *) buffer;

    while (i < length)
    {
        mod = addr % 4;

        if (mod == 0)
        {
            if (i + 4 > length)
            {
                mod = addr % 2 == 0 ? 2 : 1;
            }
        }

        if (mod == 2)
        {
            if (i + 2 > length)
            {
                mod = 1;
            }
        }

        if (mod == 0)
        {
            *((uint32_t *) addr) = ((uint32_t) ptr[i + 3]) << 24 | ((uint32_t) ptr[i + 2]) << 16 | ((uint32_t) ptr[i + 1]) << 8 | ptr[i];

            addr += 4;

            i += 4;
        }
        else if (mod == 2)
        {
            *((uint16_t *) addr) = ((uint16_t) ptr[i + 1]) << 8 | (uint16_t) ptr[i];

            addr += 2;

            i += 2;
        }
        else
        {
            *((uint8_t *) addr) = ptr[i];

            addr += 1;

            i += 1;
        }

        while ((FLASH->SR & FLASH_SR_BSY) != 0UL)
        {
            continue;
        }
    }

    if (_bc_eeprom_is_busy(10 * length))
    {
        return false;
    }

    // Disable interrupts
    bc_irq_disable();

    // Lock FLASH_PECR register
    FLASH->PECR |= FLASH_PECR_PELOCK;

    // Enable interrupts
    bc_irq_enable();

    // If we do not read what we wrote...
    if (memcmp(buffer, (void *) (address + _BC_EEPROM_BASE), length) != 0UL)
    {
        // Indicate failure
        return false;
    }

    // Indicate success
    return true;
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

    while ((FLASH->SR & FLASH_SR_BSY) != 0UL)
    {
        if (timeout > bc_tick_get())
        {
            return true;
        }
    }

    return false;
}
