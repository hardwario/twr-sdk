#include <bc_module_infra_grid.h>
#include <bc_i2c.h>
#include <bc_scheduler.h>

// Reference registers, commands
// https://na.industrial.panasonic.com/sites/default/pidsa/files/downloads/files/grid-eye-high-performance-specifications.pdf

// Adafruit Lib
// https://github.com/adafruit/Adafruit_AMG88xx/blob/master/Adafruit_AMG88xx.cpp

// interrupt flags
// https://github.com/adafruit/Adafruit_AMG88xx/blob/master/examples/amg88xx_interrupt/amg88xx_interrupt.ino

#define BC_AMG88xx_PCTL 0x00
#define BC_AMG88xx_RST 0x01
#define BC_AMG88xx_FPSC 0x02
#define BC_AMG88xx_INTC 0x03
#define BC_AMG88xx_STAT 0x04
#define BC_AMG88xx_SCLR 0x05
#define BC_AMG88xx_AVE 0x07
#define BC_AMG88xx_INTHL 0x08
#define BC_AMG88xx_TTHL 0x0E
#define BC_AMG88xx_TTHH 0x0F
#define BC_AMG88xx_INT0 0x10
#define BC_AMG88xx_AVG 0x1F
#define BC_AMG88xx_T01L 0x80

#define BC_AMG88xx_ADDR 0x68 // in 7bit

bool bc_module_infra_grid_init(bc_module_infra_grid_t *self)
{
    memset(self, 0, sizeof(*self));
    bc_i2c_init(BC_I2C_I2C0, BC_I2C_SPEED_100_KHZ);

    // Set 10 FPS and get reposnse
    bool ret = bc_i2c_memory_write_8b (BC_I2C_I2C0, BC_AMG88xx_ADDR, BC_AMG88xx_FPSC, 0x00);
    // No communication
    if (!ret)
    {
        return false;
    }

    // Diff interrpt mode, INT output reactive
    bc_i2c_memory_write_8b(BC_I2C_I2C0, BC_AMG88xx_ADDR, BC_AMG88xx_INTC, 0x00);
    // Moving average output mode active
    bc_i2c_memory_write_8b(BC_I2C_I2C0, BC_AMG88xx_ADDR, BC_AMG88xx_AVG, 0x50);
    bc_i2c_memory_write_8b(BC_I2C_I2C0, BC_AMG88xx_ADDR, BC_AMG88xx_AVG, 0x45);
    bc_i2c_memory_write_8b(BC_I2C_I2C0, BC_AMG88xx_ADDR, BC_AMG88xx_AVG, 0x57);
    bc_i2c_memory_write_8b(BC_I2C_I2C0, BC_AMG88xx_ADDR, BC_AMG88xx_AVE, 0x20);
    bc_i2c_memory_write_8b(BC_I2C_I2C0, BC_AMG88xx_ADDR, BC_AMG88xx_AVG, 0x00);

    return true;
}

float bc_module_ifra_grid_read_thermistor(bc_module_infra_grid_t *self)
{
    (void) self;
    int8_t temperature[2];

    bc_i2c_memory_read_8b(BC_I2C_I2C0, BC_AMG88xx_ADDR, BC_AMG88xx_TTHL, (uint8_t*)&temperature[0]);
    bc_i2c_memory_read_8b(BC_I2C_I2C0, BC_AMG88xx_ADDR, BC_AMG88xx_TTHH, (uint8_t*)&temperature[1]);

    return (temperature[1]*256 + temperature[0]) * 0.0625;
}


bool bc_module_ifra_grid_read_values(bc_module_infra_grid_t *self)
{
    bc_i2c_memory_transfer_t transfer;

    transfer.device_address = BC_AMG88xx_ADDR;
    transfer.memory_address = BC_AMG88xx_T01L;
    transfer.buffer = self->_sensor_data;
    transfer.length = 64 * 2;

    return bc_i2c_memory_read(BC_I2C_I2C0, &transfer);
}

void bc_module_ifra_grid_get_temperatures(bc_module_infra_grid_t *self, float *values)
{
    for (int i = 0; i < 64 ;i++)
    {
        int16_t temporary_data = self->_sensor_data[i];
        float temperature;

        if (temporary_data > 0x200)
        {
            temperature = (-temporary_data +  0xfff) * -0.25;
        }
        else
        {
            temperature = temporary_data * 0.25;
        }

        values[i] = temperature;
    }
}

