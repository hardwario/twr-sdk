#include <bc_i2c.h>
#include <bc_module_core.h>
#include <stm32l0xx.h>

static struct
{
    bool i2c0_initialized;
    bool i2c1_initialized;

    I2C_HandleTypeDef handle_i2c0;
    I2C_HandleTypeDef handle_i2c1;

} bc_i2c =
{
    .i2c0_initialized = false,
    .i2c1_initialized = false
};

void bc_i2c_init(bc_i2c_channel_t channel, bc_i2c_speed_t speed)
{
    if (channel == BC_I2C_I2C0)
    {
        if(bc_i2c.i2c0_initialized)
        {
            return;
        }

        __HAL_RCC_GPIOB_CLK_ENABLE();

        GPIO_InitTypeDef GPIO_InitStruct;

        GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF6_I2C2;

        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        __HAL_RCC_I2C2_CLK_ENABLE();

        bc_i2c.handle_i2c0.Instance = I2C2;
        if(speed == BC_I2C_SPEED_400_KHZ)
        {
            bc_i2c.handle_i2c0.Init.Timing = 0x301110;
        }
        else if (speed == BC_I2C_SPEED_100_KHZ)
        {
            bc_i2c.handle_i2c0.Init.Timing = 0x707cbb;
        }
        else
        {
            for (;;);
        }
        bc_i2c.handle_i2c0.Init.OwnAddress1 = 0;
        bc_i2c.handle_i2c0.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
        bc_i2c.handle_i2c0.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
        bc_i2c.handle_i2c0.Init.OwnAddress2 = 0;
        bc_i2c.handle_i2c0.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
        bc_i2c.handle_i2c0.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
        bc_i2c.handle_i2c0.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

        if (HAL_I2C_Init(&bc_i2c.handle_i2c0) != HAL_OK)
        {
            for (;;);
        }

        if (HAL_I2CEx_ConfigAnalogFilter(&bc_i2c.handle_i2c0, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
        {
            for (;;);
        }

        bc_i2c.i2c0_initialized = true;
    }
    else
    {
        if(bc_i2c.i2c1_initialized)
        {
            return;
        }

        // Enable PLL and disable sleep
        bc_module_core_pll_enable();

        __HAL_RCC_GPIOB_CLK_ENABLE();

        GPIO_InitTypeDef GPIO_InitStruct;

        GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;

        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        __HAL_RCC_I2C1_CLK_ENABLE();

        bc_i2c.handle_i2c1.Instance = I2C1;
        if(speed == BC_I2C_SPEED_400_KHZ)
        {
            bc_i2c.handle_i2c1.Init.Timing = 0x301110;
        }
        else if (speed == BC_I2C_SPEED_100_KHZ)
        {
            bc_i2c.handle_i2c1.Init.Timing = 0x707cbb;
        }
        else
        {
            for (;;);
        }
        bc_i2c.handle_i2c1.Init.OwnAddress1 = 0;
        bc_i2c.handle_i2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
        bc_i2c.handle_i2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
        bc_i2c.handle_i2c1.Init.OwnAddress2 = 0;
        bc_i2c.handle_i2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
        bc_i2c.handle_i2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
        bc_i2c.handle_i2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

        if (HAL_I2C_Init(&bc_i2c.handle_i2c1) != HAL_OK)
        {
            for (;;);
        }

        if (HAL_I2CEx_ConfigAnalogFilter(&bc_i2c.handle_i2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
        {
            for (;;);
        }

        bc_i2c.i2c1_initialized = true;
    }
}

bool bc_i2c_write(bc_i2c_channel_t channel, const bc_i2c_tranfer_t *transfer)
{
    // Enable PLL and disable sleep
    bc_module_core_pll_enable();

    if (channel == BC_I2C_I2C0)
    {
        if (!bc_i2c.i2c0_initialized)
        {
            return false;
        }

        if (HAL_I2C_Mem_Write(&bc_i2c.handle_i2c0, transfer->device_address << 1, transfer->memory_address, (transfer->memory_address & BC_I2C_MEMORY_ADDRESS_16_BIT) != 0 ? I2C_MEMADD_SIZE_16BIT : I2C_MEMADD_SIZE_8BIT, transfer->buffer, transfer->length, 0xFFFFFFFF) != HAL_OK)
        {
            return false;
        }

        return true;
    }
    else
    {
        if (!bc_i2c.i2c1_initialized)
        {
            return false;
        }

        if (HAL_I2C_Mem_Write(&bc_i2c.handle_i2c1, transfer->device_address << 1, transfer->memory_address, (transfer->memory_address & BC_I2C_MEMORY_ADDRESS_16_BIT) != 0 ? I2C_MEMADD_SIZE_16BIT : I2C_MEMADD_SIZE_8BIT, transfer->buffer, transfer->length, 0xFFFFFFFF) != HAL_OK)
        {
            return false;
        }

        return true;
    }

    // Disable PLL and enable sleep
    bc_module_core_pll_disable();
}

bool bc_i2c_read(bc_i2c_channel_t channel, const bc_i2c_tranfer_t *transfer)
{
    // Enable PLL and disable sleep
    bc_module_core_pll_enable();

    if (channel == BC_I2C_I2C0)
    {
        if (!bc_i2c.i2c0_initialized)
        {
            return false;
        }

        if (HAL_I2C_Mem_Read(&bc_i2c.handle_i2c0, transfer->device_address << 1, transfer->memory_address, (transfer->memory_address & BC_I2C_MEMORY_ADDRESS_16_BIT) != 0 ? I2C_MEMADD_SIZE_16BIT : I2C_MEMADD_SIZE_8BIT, transfer->buffer, transfer->length, 0xFFFFFFFF) != HAL_OK)
        {
            return false;
        }

        return true;
    }
    else
    {
        if (!bc_i2c.i2c1_initialized)
        {
            return false;
        }

        if (HAL_I2C_Mem_Read(&bc_i2c.handle_i2c1, transfer->device_address << 1, transfer->memory_address, (transfer->memory_address & BC_I2C_MEMORY_ADDRESS_16_BIT) != 0 ? I2C_MEMADD_SIZE_16BIT : I2C_MEMADD_SIZE_8BIT, transfer->buffer, transfer->length, 0xFFFFFFFF) != HAL_OK)
        {
            return false;
        }

        return true;
    }

    // Disable PLL and enable sleep
    bc_module_core_pll_disable();
}

bool bc_i2c_write_8b(bc_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint8_t data)
{
    bc_i2c_tranfer_t transfer;

    transfer.device_address = device_address;
    transfer.memory_address = memory_address;
    transfer.buffer = &data;
    transfer.length = 1;

    return bc_i2c_write(channel, &transfer);
}

bool bc_i2c_write_16b(bc_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint16_t data)
{
    uint8_t buffer[2];

    buffer[0] = data >> 8;
    buffer[1] = data;

    bc_i2c_tranfer_t transfer;

    transfer.device_address = device_address;
    transfer.memory_address = memory_address;
    transfer.buffer = buffer;
    transfer.length = 2;

    return bc_i2c_write(channel, &transfer);
}

bool bc_i2c_read_8b(bc_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint8_t *data)
{
    bc_i2c_tranfer_t transfer;

    transfer.device_address = device_address;
    transfer.memory_address = memory_address;
    transfer.buffer = data;
    transfer.length = 1;

    return bc_i2c_read(channel, &transfer);
}

bool bc_i2c_read_16b(bc_i2c_channel_t channel, uint8_t device_address, uint32_t memory_address, uint16_t *data)
{
    uint8_t buffer[2];

    bc_i2c_tranfer_t transfer;

    transfer.device_address = device_address;
    transfer.memory_address = memory_address;
    transfer.buffer = buffer;
    transfer.length = 2;

    if (!bc_i2c_read(channel, &transfer))
    {
        return false;
    }

    *data = buffer[0] << 8 | buffer[1];

    return true;
}
