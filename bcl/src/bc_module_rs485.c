#include <bc_module_rs485.h>


#define _BC_MODULE_RS485_I2C_UART_ADDRESS 0x4e
#define _BC_MODULE_RS485_I2C_TLA2021_ADDRESS 0x48

#define _BC_SC16IS7x0_REG_IER       (0x01 << 3)
#define _BC_SC16IS7X0_REG_IODIR     (0x0a << 3)
#define _BC_SC16IS7X0_REG_IOSTATE   (0x0b << 3)
#define _BC_SC16IS7X0_REG_IOINTENA  (0x0c << 3)
#define _BC_SC16IS7X0_REG_EFCR      (0x0f << 3)

#define _BC_MODULE_RS485_DELAY_RUN 50
#define _BC_MODULE_RS485_DELAY_MEASUREMENT 100

#define _BC_MODULE_RS485_ASYNC_WRITE_TASK_PERIOD 10

typedef enum
{
    BC_MODULE_RS485_STATE_ERROR = -1,
    BC_MODULE_RS485_STATE_INITIALIZE = 0,
    BC_MODULE_RS485_STATE_MEASURE = 1,
    BC_MODULE_RS485_STATE_READ = 2,
    BC_MODULE_RS485_STATE_UPDATE = 3

} bc_module_rs485_state_t;

static struct
{
    bool _initialized;
    bc_module_rs485_state_t _state;
    bc_sc16is740_t _sc16is750;

    bc_scheduler_task_id_t _task_id_measure;
    bc_scheduler_task_id_t _task_id_interval;
    bc_tick_t _update_interval;
    bc_tick_t _tick_ready;
    uint16_t _reg_result;
    bool _voltage_valid;
    bool _measurement_active;
    void (*_event_handler)(bc_module_rs485_event_t, void *);
    void *_event_param;

    bc_fifo_t *_write_fifo;
    bc_fifo_t *_read_fifo;
    bc_scheduler_task_id_t _async_write_task_id;
    bc_scheduler_task_id_t _async_read_task_id;

    bool _async_write_in_progress;
    bool _async_read_in_progress;

    uint8_t _async_buffer[64];
    bc_tick_t _async_read_timeout;

} _bc_module_rs485;

static void _bc_module_rs485_async_write_task(void *param);
static void _bc_module_rs485_async_read_task(void *param);

static void _bc_module_rs485_task_measure(void *param);
static void _bc_module_rs485_task_interval(void *param);

bool bc_module_rs485_init(void)
{
    memset(&_bc_module_rs485, 0, sizeof(_bc_module_rs485));

    if (!bc_sc16is740_init(&_bc_module_rs485._sc16is750, BC_I2C_I2C0, _BC_MODULE_RS485_I2C_UART_ADDRESS))
    {
        return false;
    }

    bc_sc16is740_reset_fifo(&_bc_module_rs485._sc16is750, BC_SC16IS740_FIFO_RX);

    // Disable sleep
    bc_i2c_memory_write_8b(BC_I2C_I2C0, _BC_MODULE_RS485_I2C_UART_ADDRESS, _BC_SC16IS7x0_REG_IER, 0x01);

    // Enable Auto RS-485 RTS output and RTS output inversion
    bc_i2c_memory_write_8b(BC_I2C_I2C0, _BC_MODULE_RS485_I2C_UART_ADDRESS, _BC_SC16IS7X0_REG_EFCR, 0x30);

    // GPIO0 set ouput (/RE)
    bc_i2c_memory_write_8b(BC_I2C_I2C0, _BC_MODULE_RS485_I2C_UART_ADDRESS, _BC_SC16IS7X0_REG_IODIR, 0x01);

    // Set GPIO0 and all other to 0 (/RE)
    bc_i2c_memory_write_8b(BC_I2C_I2C0, _BC_MODULE_RS485_I2C_UART_ADDRESS, _BC_SC16IS7X0_REG_IOSTATE, 0x00);

    _bc_module_rs485._task_id_interval = bc_scheduler_register(_bc_module_rs485_task_interval, NULL, BC_TICK_INFINITY);
    _bc_module_rs485._task_id_measure = bc_scheduler_register(_bc_module_rs485_task_measure, NULL, _BC_MODULE_RS485_DELAY_RUN);

    _bc_module_rs485._initialized = true;

    return true;
}

void bc_module_rs485_set_update_interval(bc_tick_t interval)
{
    _bc_module_rs485._update_interval = interval;

    if (_bc_module_rs485._update_interval == BC_TICK_INFINITY)
    {
        bc_scheduler_plan_absolute(_bc_module_rs485._task_id_interval, BC_TICK_INFINITY);
    }
    else
    {
        bc_scheduler_plan_relative(_bc_module_rs485._task_id_interval, _bc_module_rs485._update_interval);

        bc_module_rs485_measure();
    }
}

bool bc_module_rs485_get_voltage(float *volt)
{
    if (!_bc_module_rs485._voltage_valid)
    {
        return false;
    }

    int16_t reg_result = _bc_module_rs485._reg_result;

    if (reg_result < 0)
    {
        reg_result = 0;
    }

    reg_result >>= 4;

    *volt = 23.33f * reg_result / 2047.f;

    return true;
}

bool bc_module_rs485_measure(void)
{
    if (_bc_module_rs485._measurement_active)
    {
        return false;
    }

    _bc_module_rs485._measurement_active = true;

    bc_scheduler_plan_absolute(_bc_module_rs485._task_id_measure, _bc_module_rs485._tick_ready);

    return true;
}

static void _bc_module_rs485_async_write_task(void *param)
{
    (void) param;

    size_t space_available;

    if (bc_fifo_is_empty(_bc_module_rs485._write_fifo))
    {
        bc_scheduler_unregister(_bc_module_rs485._async_write_task_id);
        _bc_module_rs485._async_write_in_progress = false;

        _bc_module_rs485._event_handler(BC_MODULE_RS485_EVENT_ASYNC_WRITE_DONE, _bc_module_rs485._event_param);

        return;
    }

    if (!bc_sc16is740_get_spaces_available(&_bc_module_rs485._sc16is750, &space_available))
    {
        bc_scheduler_unregister(_bc_module_rs485._async_write_task_id);
        _bc_module_rs485._async_write_in_progress = false;

        _bc_module_rs485._event_handler(BC_MODULE_RS485_EVENT_ERROR, _bc_module_rs485._event_param);
        return;
    }

    size_t bytes_read = bc_fifo_read(_bc_module_rs485._write_fifo, _bc_module_rs485._async_buffer, space_available);
    bc_module_rs485_write(_bc_module_rs485._async_buffer, bytes_read);

    bc_scheduler_plan_current_relative(_BC_MODULE_RS485_ASYNC_WRITE_TASK_PERIOD);
}

static void _bc_module_rs485_async_read_task(void *param)
{
    (void) param;

    size_t available = 0;

    if (!bc_sc16is740_available(&_bc_module_rs485._sc16is750, &available))
    {
        return;
    }

    if (available)
    {
        bc_sc16is740_read(&_bc_module_rs485._sc16is750, _bc_module_rs485._async_buffer, available, 0);
        bc_fifo_write(_bc_module_rs485._read_fifo, _bc_module_rs485._async_buffer, available);
    }

    if (!bc_fifo_is_empty(_bc_module_rs485._read_fifo))
    {
        _bc_module_rs485._event_handler(BC_MODULE_RS485_EVENT_ASYNC_READ_DATA, _bc_module_rs485._event_param);
    }
    else
    {
        _bc_module_rs485._event_handler(BC_MODULE_RS485_EVENT_ASYNC_READ_TIMEOUT, _bc_module_rs485._event_param);
    }

    bc_scheduler_plan_current_relative(_bc_module_rs485._async_read_timeout);
}

static void _bc_module_rs485_task_interval(void *param)
{
    (void) param;

    bc_module_rs485_measure();
    bc_scheduler_plan_current_relative(_bc_module_rs485._update_interval);
}

static void _bc_module_rs485_task_measure(void *param)
{
    (void) param;

    start:

    switch (_bc_module_rs485._state)
    {
        case BC_MODULE_RS485_STATE_ERROR:
        {
            if (_bc_module_rs485._event_handler != NULL)
            {
                _bc_module_rs485._event_handler(BC_MODULE_RS485_EVENT_ERROR, _bc_module_rs485._event_param);
            }

            _bc_module_rs485._state = BC_MODULE_RS485_STATE_INITIALIZE;

            return;
        }
        case BC_MODULE_RS485_STATE_INITIALIZE:
        {
            _bc_module_rs485._state = BC_MODULE_RS485_STATE_ERROR;

            if (!bc_i2c_memory_write_16b(BC_I2C_I2C0, _BC_MODULE_RS485_I2C_TLA2021_ADDRESS, 0x01, 0x0503))
            {
                goto start;
            }

            _bc_module_rs485._state = BC_MODULE_RS485_STATE_MEASURE;

            _bc_module_rs485._tick_ready = bc_tick_get();

            if (_bc_module_rs485._measurement_active)
            {
                bc_scheduler_plan_current_absolute(_bc_module_rs485._tick_ready);
            }

            return;
        }
        case BC_MODULE_RS485_STATE_MEASURE:
        {
            _bc_module_rs485._state = BC_MODULE_RS485_STATE_ERROR;

            if (!bc_i2c_memory_write_16b(BC_I2C_I2C0, _BC_MODULE_RS485_I2C_TLA2021_ADDRESS, 0x01, 0x8503))
            {
                goto start;
            }

            _bc_module_rs485._state = BC_MODULE_RS485_STATE_READ;

            bc_scheduler_plan_current_from_now(_BC_MODULE_RS485_DELAY_MEASUREMENT);

            return;
        }
        case BC_MODULE_RS485_STATE_READ:
        {
            _bc_module_rs485._state = BC_MODULE_RS485_STATE_ERROR;

            uint16_t reg_configuration;

            if (!bc_i2c_memory_read_16b(BC_I2C_I2C0, _BC_MODULE_RS485_I2C_TLA2021_ADDRESS, 0x01, &reg_configuration))
            {
                goto start;
            }

            if ((reg_configuration & 0x8000) != 0x8000)
            {
                goto start;
            }

            if (!bc_i2c_memory_read_16b(BC_I2C_I2C0, _BC_MODULE_RS485_I2C_TLA2021_ADDRESS, 0x00, &_bc_module_rs485._reg_result))
            {
                goto start;
            }

            _bc_module_rs485._voltage_valid = true;

            _bc_module_rs485._state = BC_MODULE_RS485_STATE_UPDATE;

            goto start;
        }
        case BC_MODULE_RS485_STATE_UPDATE:
        {
            _bc_module_rs485._measurement_active = false;

            if (_bc_module_rs485._event_handler != NULL)
            {
                _bc_module_rs485._event_handler(BC_MODULE_RS485_EVENT_VOLTAGE, _bc_module_rs485._event_param);
            }

            _bc_module_rs485._state = BC_MODULE_RS485_STATE_MEASURE;

            return;
        }
        default:
        {
            _bc_module_rs485._state = BC_MODULE_RS485_STATE_ERROR;

            goto start;
        }
    }
}

void bc_module_rs485_set_async_fifo(bc_fifo_t *write_fifo, bc_fifo_t *read_fifo)
{
    _bc_module_rs485._write_fifo = write_fifo;
    _bc_module_rs485._read_fifo = read_fifo;
}

size_t bc_module_rs485_async_write(uint8_t *buffer, size_t length)
{
    if (!_bc_module_rs485._initialized || _bc_module_rs485._write_fifo == NULL)
    {
        return 0;
    }

    size_t bytes_written = bc_fifo_write(_bc_module_rs485._write_fifo, (uint8_t *) buffer, length);

    if (bytes_written != 0)
    {
        if (!_bc_module_rs485._async_write_in_progress)
        {
            _bc_module_rs485._async_write_task_id = bc_scheduler_register(_bc_module_rs485_async_write_task, NULL, 10);
            _bc_module_rs485._async_write_in_progress = true;
        }
    }

    return bytes_written;
}

bool bc_module_rs485_async_read_start(bc_tick_t timeout)
{
    if (!_bc_module_rs485._initialized || _bc_module_rs485._read_fifo == NULL || _bc_module_rs485._async_read_in_progress)
    {
        return false;
    }

    _bc_module_rs485._async_read_timeout = timeout;
    _bc_module_rs485._async_read_task_id = bc_scheduler_register(_bc_module_rs485_async_read_task, NULL, _bc_module_rs485._async_read_timeout);
    _bc_module_rs485._async_read_in_progress = true;

    return true;
}

bool bc_module_rs485_async_read_stop(void)
{
    if (!_bc_module_rs485._initialized || !_bc_module_rs485._async_read_in_progress)
    {
        return false;
    }

    _bc_module_rs485._async_read_in_progress = false;
    bc_scheduler_unregister(_bc_module_rs485._async_read_task_id);

    return true;
}

size_t bc_module_rs485_async_read(void *buffer, size_t length)
{
    if (!_bc_module_rs485._initialized || _bc_module_rs485._read_fifo == NULL || !_bc_module_rs485._async_read_in_progress)
    {
        return 0;
    }

    return bc_fifo_read(_bc_module_rs485._read_fifo, buffer, length);
}

void bc_module_rs485_set_event_handler(void (*event_handler)(bc_module_rs485_event_t, void *), void *event_param)
{
    _bc_module_rs485._event_handler = event_handler;
    _bc_module_rs485._event_param = event_param;
}

size_t bc_module_rs485_write(uint8_t *buffer, size_t length)
{
    return bc_sc16is740_write(&_bc_module_rs485._sc16is750, buffer, length);
}

bool bc_module_rs485_available(size_t *available)
{
    return bc_sc16is740_available(&_bc_module_rs485._sc16is750, available);
}

size_t bc_module_rs485_read(uint8_t *buffer, size_t length, bc_tick_t timeout)
{
    return bc_sc16is740_read(&_bc_module_rs485._sc16is750, buffer, length, timeout);
}

bool bc_module_rs485_set_baudrate(bc_module_rs485_baudrate_t baudrate)
{
    return bc_sc16is740_set_baudrate(&_bc_module_rs485._sc16is750, baudrate);
}

