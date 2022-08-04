#include <twr_module_rs485.h>


#define _TWR_MODULE_RS485_I2C_UART_ADDRESS 0x4e
#define _TWR_MODULE_RS485_I2C_TLA2021_ADDRESS 0x48

#define _TWR_SC16IS7x0_REG_IER       (0x01 << 3)
#define _TWR_SC16IS7X0_REG_IODIR     (0x0a << 3)
#define _TWR_SC16IS7X0_REG_IOSTATE   (0x0b << 3)
#define _TWR_SC16IS7X0_REG_IOINTENA  (0x0c << 3)
#define _TWR_SC16IS7X0_REG_EFCR      (0x0f << 3)

#define _TWR_MODULE_RS485_DELAY_RUN 50
#define _TWR_MODULE_RS485_DELAY_MEASUREMENT 100

#define _TWR_MODULE_RS485_ASYNC_WRITE_TASK_PERIOD 10

typedef enum
{
    TWR_MODULE_RS485_STATE_ERROR = -1,
    TWR_MODULE_RS485_STATE_INITIALIZE = 0,
    TWR_MODULE_RS485_STATE_MEASURE = 1,
    TWR_MODULE_RS485_STATE_READ = 2,
    TWR_MODULE_RS485_STATE_UPDATE = 3

} twr_module_rs485_state_t;

static struct
{
    bool _initialized;
    twr_module_rs485_state_t _state;
    twr_sc16is740_t _sc16is750;

    twr_scheduler_task_id_t _task_id_measure;
    twr_scheduler_task_id_t _task_id_interval;
    twr_tick_t _update_interval;
    twr_tick_t _tick_ready;
    uint16_t _reg_result;
    bool _voltage_valid;
    bool _measurement_active;
    void (*_event_handler)(twr_module_rs485_event_t, void *);
    void *_event_param;

    twr_fifo_t *_write_fifo;
    twr_fifo_t *_read_fifo;
    twr_scheduler_task_id_t _async_write_task_id;
    twr_scheduler_task_id_t _async_read_task_id;

    bool _async_write_in_progress;
    bool _async_read_in_progress;

    uint8_t _async_buffer[64];
    twr_tick_t _async_read_timeout;

} _twr_module_rs485;

static void _twr_module_rs485_async_write_task(void *param);
static void _twr_module_rs485_async_read_task(void *param);

static void _twr_module_rs485_task_measure(void *param);
static void _twr_module_rs485_task_interval(void *param);

bool twr_module_rs485_init(void)
{
    memset(&_twr_module_rs485, 0, sizeof(_twr_module_rs485));

    if (!twr_sc16is740_init(&_twr_module_rs485._sc16is750, TWR_I2C_I2C0, _TWR_MODULE_RS485_I2C_UART_ADDRESS))
    {
        return false;
    }

    if (!twr_sc16is740_reset_fifo(&_twr_module_rs485._sc16is750, TWR_SC16IS740_FIFO_RX))
    {
        return false;
    }

    // Disable sleep
    if (!twr_i2c_memory_write_8b(TWR_I2C_I2C0, _TWR_MODULE_RS485_I2C_UART_ADDRESS, _TWR_SC16IS7x0_REG_IER, 0x01))
    {
        return false;
    }

    // Enable Auto RS-485 RTS output and RTS output inversion
    if (!twr_i2c_memory_write_8b(TWR_I2C_I2C0, _TWR_MODULE_RS485_I2C_UART_ADDRESS, _TWR_SC16IS7X0_REG_EFCR, 0x30))
    {
        return false;
    }

    // GPIO0 set ouput (/RE)
    if (!twr_i2c_memory_write_8b(TWR_I2C_I2C0, _TWR_MODULE_RS485_I2C_UART_ADDRESS, _TWR_SC16IS7X0_REG_IODIR, 0x01))
    {
        return false;
    }

    // Set GPIO0 and all other to 0 (/RE)
    if (!twr_i2c_memory_write_8b(TWR_I2C_I2C0, _TWR_MODULE_RS485_I2C_UART_ADDRESS, _TWR_SC16IS7X0_REG_IOSTATE, 0x00))
    {
        return false;
    }

    _twr_module_rs485._task_id_interval = twr_scheduler_register(_twr_module_rs485_task_interval, NULL, TWR_TICK_INFINITY);
    _twr_module_rs485._task_id_measure = twr_scheduler_register(_twr_module_rs485_task_measure, NULL, _TWR_MODULE_RS485_DELAY_RUN);

    _twr_module_rs485._initialized = true;

    return true;
}


bool twr_module_rs485_deinit(void)
{
    if (_twr_module_rs485._initialized)
    {
        twr_scheduler_unregister(_twr_module_rs485._task_id_interval);
        twr_scheduler_unregister(_twr_module_rs485._task_id_measure);
    }

    _twr_module_rs485._initialized = false;

    // Enable sleep
    twr_i2c_memory_write_8b(TWR_I2C_I2C0, _TWR_MODULE_RS485_I2C_UART_ADDRESS, _TWR_SC16IS7x0_REG_IER, 0x00);
    {
        return false;
    }

    return true;
}


void twr_module_rs485_set_update_interval(twr_tick_t interval)
{
    _twr_module_rs485._update_interval = interval;

    if (_twr_module_rs485._update_interval == TWR_TICK_INFINITY)
    {
        twr_scheduler_plan_absolute(_twr_module_rs485._task_id_interval, TWR_TICK_INFINITY);
    }
    else
    {
        twr_scheduler_plan_relative(_twr_module_rs485._task_id_interval, _twr_module_rs485._update_interval);

        twr_module_rs485_measure();
    }
}

bool twr_module_rs485_get_voltage(float *volt)
{
    if (!_twr_module_rs485._voltage_valid)
    {
        return false;
    }

    int16_t reg_result = _twr_module_rs485._reg_result;

    if (reg_result < 0)
    {
        reg_result = 0;
    }

    reg_result >>= 4;

    *volt = 39.62f * reg_result / 2047.f;

    return true;
}

bool twr_module_rs485_measure(void)
{
    if (_twr_module_rs485._measurement_active)
    {
        return false;
    }

    _twr_module_rs485._measurement_active = true;

    twr_scheduler_plan_absolute(_twr_module_rs485._task_id_measure, _twr_module_rs485._tick_ready);

    return true;
}

static void _twr_module_rs485_async_write_task(void *param)
{
    (void) param;

    size_t space_available;

    if (twr_fifo_is_empty(_twr_module_rs485._write_fifo))
    {
        twr_scheduler_unregister(_twr_module_rs485._async_write_task_id);
        _twr_module_rs485._async_write_in_progress = false;

        _twr_module_rs485._event_handler(TWR_MODULE_RS485_EVENT_ASYNC_WRITE_DONE, _twr_module_rs485._event_param);

        return;
    }

    if (!twr_sc16is740_get_spaces_available(&_twr_module_rs485._sc16is750, &space_available))
    {
        twr_scheduler_unregister(_twr_module_rs485._async_write_task_id);
        _twr_module_rs485._async_write_in_progress = false;

        _twr_module_rs485._event_handler(TWR_MODULE_RS485_EVENT_ERROR, _twr_module_rs485._event_param);
        return;
    }

    size_t bytes_read = twr_fifo_read(_twr_module_rs485._write_fifo, _twr_module_rs485._async_buffer, space_available);
    twr_module_rs485_write(_twr_module_rs485._async_buffer, bytes_read);

    twr_scheduler_plan_current_relative(_TWR_MODULE_RS485_ASYNC_WRITE_TASK_PERIOD);
}

static void _twr_module_rs485_async_read_task(void *param)
{
    (void) param;

    size_t available = 0;

    if (!twr_sc16is740_available(&_twr_module_rs485._sc16is750, &available))
    {
        return;
    }

    if (available)
    {
        twr_sc16is740_read(&_twr_module_rs485._sc16is750, _twr_module_rs485._async_buffer, available, 0);
        twr_fifo_write(_twr_module_rs485._read_fifo, _twr_module_rs485._async_buffer, available);
    }

    if (!twr_fifo_is_empty(_twr_module_rs485._read_fifo))
    {
        _twr_module_rs485._event_handler(TWR_MODULE_RS485_EVENT_ASYNC_READ_DATA, _twr_module_rs485._event_param);
    }
    else
    {
        _twr_module_rs485._event_handler(TWR_MODULE_RS485_EVENT_ASYNC_READ_TIMEOUT, _twr_module_rs485._event_param);
    }

    twr_scheduler_plan_current_relative(_twr_module_rs485._async_read_timeout);
}

static void _twr_module_rs485_task_interval(void *param)
{
    (void) param;

    twr_module_rs485_measure();
    twr_scheduler_plan_current_relative(_twr_module_rs485._update_interval);
}

static void _twr_module_rs485_task_measure(void *param)
{
    (void) param;

    start:

    switch (_twr_module_rs485._state)
    {
        case TWR_MODULE_RS485_STATE_ERROR:
        {
            if (_twr_module_rs485._event_handler != NULL)
            {
                _twr_module_rs485._event_handler(TWR_MODULE_RS485_EVENT_ERROR, _twr_module_rs485._event_param);
            }

            _twr_module_rs485._state = TWR_MODULE_RS485_STATE_INITIALIZE;

            return;
        }
        case TWR_MODULE_RS485_STATE_INITIALIZE:
        {
            _twr_module_rs485._state = TWR_MODULE_RS485_STATE_ERROR;

            if (!twr_i2c_memory_write_16b(TWR_I2C_I2C0, _TWR_MODULE_RS485_I2C_TLA2021_ADDRESS, 0x01, 0x0503))
            {
                goto start;
            }

            _twr_module_rs485._state = TWR_MODULE_RS485_STATE_MEASURE;

            _twr_module_rs485._tick_ready = twr_tick_get();

            if (_twr_module_rs485._measurement_active)
            {
                twr_scheduler_plan_current_absolute(_twr_module_rs485._tick_ready);
            }

            return;
        }
        case TWR_MODULE_RS485_STATE_MEASURE:
        {
            _twr_module_rs485._state = TWR_MODULE_RS485_STATE_ERROR;

            if (!twr_i2c_memory_write_16b(TWR_I2C_I2C0, _TWR_MODULE_RS485_I2C_TLA2021_ADDRESS, 0x01, 0x8503))
            {
                goto start;
            }

            _twr_module_rs485._state = TWR_MODULE_RS485_STATE_READ;

            twr_scheduler_plan_current_from_now(_TWR_MODULE_RS485_DELAY_MEASUREMENT);

            return;
        }
        case TWR_MODULE_RS485_STATE_READ:
        {
            _twr_module_rs485._state = TWR_MODULE_RS485_STATE_ERROR;

            uint16_t reg_configuration;

            if (!twr_i2c_memory_read_16b(TWR_I2C_I2C0, _TWR_MODULE_RS485_I2C_TLA2021_ADDRESS, 0x01, &reg_configuration))
            {
                goto start;
            }

            if ((reg_configuration & 0x8000) != 0x8000)
            {
                goto start;
            }

            if (!twr_i2c_memory_read_16b(TWR_I2C_I2C0, _TWR_MODULE_RS485_I2C_TLA2021_ADDRESS, 0x00, &_twr_module_rs485._reg_result))
            {
                goto start;
            }

            _twr_module_rs485._voltage_valid = true;

            _twr_module_rs485._state = TWR_MODULE_RS485_STATE_UPDATE;

            goto start;
        }
        case TWR_MODULE_RS485_STATE_UPDATE:
        {
            _twr_module_rs485._measurement_active = false;

            if (_twr_module_rs485._event_handler != NULL)
            {
                _twr_module_rs485._event_handler(TWR_MODULE_RS485_EVENT_VOLTAGE, _twr_module_rs485._event_param);
            }

            _twr_module_rs485._state = TWR_MODULE_RS485_STATE_MEASURE;

            return;
        }
        default:
        {
            _twr_module_rs485._state = TWR_MODULE_RS485_STATE_ERROR;

            goto start;
        }
    }
}

void twr_module_rs485_set_async_fifo(twr_fifo_t *write_fifo, twr_fifo_t *read_fifo)
{
    _twr_module_rs485._write_fifo = write_fifo;
    _twr_module_rs485._read_fifo = read_fifo;
}

size_t twr_module_rs485_async_write(uint8_t *buffer, size_t length)
{
    if (!_twr_module_rs485._initialized || _twr_module_rs485._write_fifo == NULL)
    {
        return 0;
    }

    size_t bytes_written = twr_fifo_write(_twr_module_rs485._write_fifo, (uint8_t *) buffer, length);

    if (bytes_written != 0)
    {
        if (!_twr_module_rs485._async_write_in_progress)
        {
            _twr_module_rs485._async_write_task_id = twr_scheduler_register(_twr_module_rs485_async_write_task, NULL, 10);
            _twr_module_rs485._async_write_in_progress = true;
        }
    }

    return bytes_written;
}

bool twr_module_rs485_async_read_start(twr_tick_t timeout)
{
    if (!_twr_module_rs485._initialized || _twr_module_rs485._read_fifo == NULL || _twr_module_rs485._async_read_in_progress)
    {
        return false;
    }

    _twr_module_rs485._async_read_timeout = timeout;
    _twr_module_rs485._async_read_task_id = twr_scheduler_register(_twr_module_rs485_async_read_task, NULL, _twr_module_rs485._async_read_timeout);
    _twr_module_rs485._async_read_in_progress = true;

    return true;
}

bool twr_module_rs485_async_read_stop(void)
{
    if (!_twr_module_rs485._initialized || !_twr_module_rs485._async_read_in_progress)
    {
        return false;
    }

    _twr_module_rs485._async_read_in_progress = false;
    twr_scheduler_unregister(_twr_module_rs485._async_read_task_id);

    return true;
}

size_t twr_module_rs485_async_read(void *buffer, size_t length)
{
    if (!_twr_module_rs485._initialized || _twr_module_rs485._read_fifo == NULL || !_twr_module_rs485._async_read_in_progress)
    {
        return 0;
    }

    return twr_fifo_read(_twr_module_rs485._read_fifo, buffer, length);
}

void twr_module_rs485_set_event_handler(void (*event_handler)(twr_module_rs485_event_t, void *), void *event_param)
{
    _twr_module_rs485._event_handler = event_handler;
    _twr_module_rs485._event_param = event_param;
}

size_t twr_module_rs485_write(uint8_t *buffer, size_t length)
{
    return twr_sc16is740_write(&_twr_module_rs485._sc16is750, buffer, length);
}

bool twr_module_rs485_available(size_t *available)
{
    return twr_sc16is740_available(&_twr_module_rs485._sc16is750, available);
}

size_t twr_module_rs485_read(uint8_t *buffer, size_t length, twr_tick_t timeout)
{
    return twr_sc16is740_read(&_twr_module_rs485._sc16is750, buffer, length, timeout);
}

bool twr_module_rs485_set_baudrate(twr_module_rs485_baudrate_t baudrate)
{
    return twr_sc16is740_set_baudrate(&_twr_module_rs485._sc16is750, (twr_sc16is740_baudrate_t) baudrate);
}

