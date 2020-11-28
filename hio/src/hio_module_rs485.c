#include <hio_module_rs485.h>


#define _HIO_MODULE_RS485_I2C_UART_ADDRESS 0x4e
#define _HIO_MODULE_RS485_I2C_TLA2021_ADDRESS 0x48

#define _HIO_SC16IS7x0_REG_IER       (0x01 << 3)
#define _HIO_SC16IS7X0_REG_IODIR     (0x0a << 3)
#define _HIO_SC16IS7X0_REG_IOSTATE   (0x0b << 3)
#define _HIO_SC16IS7X0_REG_IOINTENA  (0x0c << 3)
#define _HIO_SC16IS7X0_REG_EFCR      (0x0f << 3)

#define _HIO_MODULE_RS485_DELAY_RUN 50
#define _HIO_MODULE_RS485_DELAY_MEASUREMENT 100

#define _HIO_MODULE_RS485_ASYNC_WRITE_TASK_PERIOD 10

typedef enum
{
    HIO_MODULE_RS485_STATE_ERROR = -1,
    HIO_MODULE_RS485_STATE_INITIALIZE = 0,
    HIO_MODULE_RS485_STATE_MEASURE = 1,
    HIO_MODULE_RS485_STATE_READ = 2,
    HIO_MODULE_RS485_STATE_UPDATE = 3

} hio_module_rs485_state_t;

static struct
{
    bool _initialized;
    hio_module_rs485_state_t _state;
    hio_sc16is740_t _sc16is750;

    hio_scheduler_task_id_t _task_id_measure;
    hio_scheduler_task_id_t _task_id_interval;
    hio_tick_t _update_interval;
    hio_tick_t _tick_ready;
    uint16_t _reg_result;
    bool _voltage_valid;
    bool _measurement_active;
    void (*_event_handler)(hio_module_rs485_event_t, void *);
    void *_event_param;

    hio_fifo_t *_write_fifo;
    hio_fifo_t *_read_fifo;
    hio_scheduler_task_id_t _async_write_task_id;
    hio_scheduler_task_id_t _async_read_task_id;

    bool _async_write_in_progress;
    bool _async_read_in_progress;

    uint8_t _async_buffer[64];
    hio_tick_t _async_read_timeout;

} _hio_module_rs485;

static void _hio_module_rs485_async_write_task(void *param);
static void _hio_module_rs485_async_read_task(void *param);

static void _hio_module_rs485_task_measure(void *param);
static void _hio_module_rs485_task_interval(void *param);

bool hio_module_rs485_init(void)
{
    memset(&_hio_module_rs485, 0, sizeof(_hio_module_rs485));

    if (!hio_sc16is740_init(&_hio_module_rs485._sc16is750, HIO_I2C_I2C0, _HIO_MODULE_RS485_I2C_UART_ADDRESS))
    {
        return false;
    }

    if (!hio_sc16is740_reset_fifo(&_hio_module_rs485._sc16is750, HIO_SC16IS740_FIFO_RX))
    {
        return false;
    }

    // Disable sleep
    if (!hio_i2c_memory_write_8b(HIO_I2C_I2C0, _HIO_MODULE_RS485_I2C_UART_ADDRESS, _HIO_SC16IS7x0_REG_IER, 0x01))
    {
        return false;
    }

    // Enable Auto RS-485 RTS output and RTS output inversion
    if (!hio_i2c_memory_write_8b(HIO_I2C_I2C0, _HIO_MODULE_RS485_I2C_UART_ADDRESS, _HIO_SC16IS7X0_REG_EFCR, 0x30))
    {
        return false;
    }

    // GPIO0 set ouput (/RE)
    if (!hio_i2c_memory_write_8b(HIO_I2C_I2C0, _HIO_MODULE_RS485_I2C_UART_ADDRESS, _HIO_SC16IS7X0_REG_IODIR, 0x01))
    {
        return false;
    }

    // Set GPIO0 and all other to 0 (/RE)
    if (!hio_i2c_memory_write_8b(HIO_I2C_I2C0, _HIO_MODULE_RS485_I2C_UART_ADDRESS, _HIO_SC16IS7X0_REG_IOSTATE, 0x00))
    {
        return false;
    }

    _hio_module_rs485._task_id_interval = hio_scheduler_register(_hio_module_rs485_task_interval, NULL, HIO_TICK_INFINITY);
    _hio_module_rs485._task_id_measure = hio_scheduler_register(_hio_module_rs485_task_measure, NULL, _HIO_MODULE_RS485_DELAY_RUN);

    _hio_module_rs485._initialized = true;

    return true;
}


bool hio_module_rs485_deinit(void)
{
    if (_hio_module_rs485._initialized)
    {
        hio_scheduler_unregister(_hio_module_rs485._task_id_interval);
        hio_scheduler_unregister(_hio_module_rs485._task_id_measure);
    }

    _hio_module_rs485._initialized = false;

    // Enable sleep
    hio_i2c_memory_write_8b(HIO_I2C_I2C0, _HIO_MODULE_RS485_I2C_UART_ADDRESS, _HIO_SC16IS7x0_REG_IER, 0x00);
    {
        return false;
    }

    return true;
}


void hio_module_rs485_set_update_interval(hio_tick_t interval)
{
    _hio_module_rs485._update_interval = interval;

    if (_hio_module_rs485._update_interval == HIO_TICK_INFINITY)
    {
        hio_scheduler_plan_absolute(_hio_module_rs485._task_id_interval, HIO_TICK_INFINITY);
    }
    else
    {
        hio_scheduler_plan_relative(_hio_module_rs485._task_id_interval, _hio_module_rs485._update_interval);

        hio_module_rs485_measure();
    }
}

bool hio_module_rs485_get_voltage(float *volt)
{
    if (!_hio_module_rs485._voltage_valid)
    {
        return false;
    }

    int16_t reg_result = _hio_module_rs485._reg_result;

    if (reg_result < 0)
    {
        reg_result = 0;
    }

    reg_result >>= 4;

    *volt = 39.62f * reg_result / 2047.f;

    return true;
}

bool hio_module_rs485_measure(void)
{
    if (_hio_module_rs485._measurement_active)
    {
        return false;
    }

    _hio_module_rs485._measurement_active = true;

    hio_scheduler_plan_absolute(_hio_module_rs485._task_id_measure, _hio_module_rs485._tick_ready);

    return true;
}

static void _hio_module_rs485_async_write_task(void *param)
{
    (void) param;

    size_t space_available;

    if (hio_fifo_is_empty(_hio_module_rs485._write_fifo))
    {
        hio_scheduler_unregister(_hio_module_rs485._async_write_task_id);
        _hio_module_rs485._async_write_in_progress = false;

        _hio_module_rs485._event_handler(HIO_MODULE_RS485_EVENT_ASYNC_WRITE_DONE, _hio_module_rs485._event_param);

        return;
    }

    if (!hio_sc16is740_get_spaces_available(&_hio_module_rs485._sc16is750, &space_available))
    {
        hio_scheduler_unregister(_hio_module_rs485._async_write_task_id);
        _hio_module_rs485._async_write_in_progress = false;

        _hio_module_rs485._event_handler(HIO_MODULE_RS485_EVENT_ERROR, _hio_module_rs485._event_param);
        return;
    }

    size_t bytes_read = hio_fifo_read(_hio_module_rs485._write_fifo, _hio_module_rs485._async_buffer, space_available);
    hio_module_rs485_write(_hio_module_rs485._async_buffer, bytes_read);

    hio_scheduler_plan_current_relative(_HIO_MODULE_RS485_ASYNC_WRITE_TASK_PERIOD);
}

static void _hio_module_rs485_async_read_task(void *param)
{
    (void) param;

    size_t available = 0;

    if (!hio_sc16is740_available(&_hio_module_rs485._sc16is750, &available))
    {
        return;
    }

    if (available)
    {
        hio_sc16is740_read(&_hio_module_rs485._sc16is750, _hio_module_rs485._async_buffer, available, 0);
        hio_fifo_write(_hio_module_rs485._read_fifo, _hio_module_rs485._async_buffer, available);
    }

    if (!hio_fifo_is_empty(_hio_module_rs485._read_fifo))
    {
        _hio_module_rs485._event_handler(HIO_MODULE_RS485_EVENT_ASYNC_READ_DATA, _hio_module_rs485._event_param);
    }
    else
    {
        _hio_module_rs485._event_handler(HIO_MODULE_RS485_EVENT_ASYNC_READ_TIMEOUT, _hio_module_rs485._event_param);
    }

    hio_scheduler_plan_current_relative(_hio_module_rs485._async_read_timeout);
}

static void _hio_module_rs485_task_interval(void *param)
{
    (void) param;

    hio_module_rs485_measure();
    hio_scheduler_plan_current_relative(_hio_module_rs485._update_interval);
}

static void _hio_module_rs485_task_measure(void *param)
{
    (void) param;

    start:

    switch (_hio_module_rs485._state)
    {
        case HIO_MODULE_RS485_STATE_ERROR:
        {
            if (_hio_module_rs485._event_handler != NULL)
            {
                _hio_module_rs485._event_handler(HIO_MODULE_RS485_EVENT_ERROR, _hio_module_rs485._event_param);
            }

            _hio_module_rs485._state = HIO_MODULE_RS485_STATE_INITIALIZE;

            return;
        }
        case HIO_MODULE_RS485_STATE_INITIALIZE:
        {
            _hio_module_rs485._state = HIO_MODULE_RS485_STATE_ERROR;

            if (!hio_i2c_memory_write_16b(HIO_I2C_I2C0, _HIO_MODULE_RS485_I2C_TLA2021_ADDRESS, 0x01, 0x0503))
            {
                goto start;
            }

            _hio_module_rs485._state = HIO_MODULE_RS485_STATE_MEASURE;

            _hio_module_rs485._tick_ready = hio_tick_get();

            if (_hio_module_rs485._measurement_active)
            {
                hio_scheduler_plan_current_absolute(_hio_module_rs485._tick_ready);
            }

            return;
        }
        case HIO_MODULE_RS485_STATE_MEASURE:
        {
            _hio_module_rs485._state = HIO_MODULE_RS485_STATE_ERROR;

            if (!hio_i2c_memory_write_16b(HIO_I2C_I2C0, _HIO_MODULE_RS485_I2C_TLA2021_ADDRESS, 0x01, 0x8503))
            {
                goto start;
            }

            _hio_module_rs485._state = HIO_MODULE_RS485_STATE_READ;

            hio_scheduler_plan_current_from_now(_HIO_MODULE_RS485_DELAY_MEASUREMENT);

            return;
        }
        case HIO_MODULE_RS485_STATE_READ:
        {
            _hio_module_rs485._state = HIO_MODULE_RS485_STATE_ERROR;

            uint16_t reg_configuration;

            if (!hio_i2c_memory_read_16b(HIO_I2C_I2C0, _HIO_MODULE_RS485_I2C_TLA2021_ADDRESS, 0x01, &reg_configuration))
            {
                goto start;
            }

            if ((reg_configuration & 0x8000) != 0x8000)
            {
                goto start;
            }

            if (!hio_i2c_memory_read_16b(HIO_I2C_I2C0, _HIO_MODULE_RS485_I2C_TLA2021_ADDRESS, 0x00, &_hio_module_rs485._reg_result))
            {
                goto start;
            }

            _hio_module_rs485._voltage_valid = true;

            _hio_module_rs485._state = HIO_MODULE_RS485_STATE_UPDATE;

            goto start;
        }
        case HIO_MODULE_RS485_STATE_UPDATE:
        {
            _hio_module_rs485._measurement_active = false;

            if (_hio_module_rs485._event_handler != NULL)
            {
                _hio_module_rs485._event_handler(HIO_MODULE_RS485_EVENT_VOLTAGE, _hio_module_rs485._event_param);
            }

            _hio_module_rs485._state = HIO_MODULE_RS485_STATE_MEASURE;

            return;
        }
        default:
        {
            _hio_module_rs485._state = HIO_MODULE_RS485_STATE_ERROR;

            goto start;
        }
    }
}

void hio_module_rs485_set_async_fifo(hio_fifo_t *write_fifo, hio_fifo_t *read_fifo)
{
    _hio_module_rs485._write_fifo = write_fifo;
    _hio_module_rs485._read_fifo = read_fifo;
}

size_t hio_module_rs485_async_write(uint8_t *buffer, size_t length)
{
    if (!_hio_module_rs485._initialized || _hio_module_rs485._write_fifo == NULL)
    {
        return 0;
    }

    size_t bytes_written = hio_fifo_write(_hio_module_rs485._write_fifo, (uint8_t *) buffer, length);

    if (bytes_written != 0)
    {
        if (!_hio_module_rs485._async_write_in_progress)
        {
            _hio_module_rs485._async_write_task_id = hio_scheduler_register(_hio_module_rs485_async_write_task, NULL, 10);
            _hio_module_rs485._async_write_in_progress = true;
        }
    }

    return bytes_written;
}

bool hio_module_rs485_async_read_start(hio_tick_t timeout)
{
    if (!_hio_module_rs485._initialized || _hio_module_rs485._read_fifo == NULL || _hio_module_rs485._async_read_in_progress)
    {
        return false;
    }

    _hio_module_rs485._async_read_timeout = timeout;
    _hio_module_rs485._async_read_task_id = hio_scheduler_register(_hio_module_rs485_async_read_task, NULL, _hio_module_rs485._async_read_timeout);
    _hio_module_rs485._async_read_in_progress = true;

    return true;
}

bool hio_module_rs485_async_read_stop(void)
{
    if (!_hio_module_rs485._initialized || !_hio_module_rs485._async_read_in_progress)
    {
        return false;
    }

    _hio_module_rs485._async_read_in_progress = false;
    hio_scheduler_unregister(_hio_module_rs485._async_read_task_id);

    return true;
}

size_t hio_module_rs485_async_read(void *buffer, size_t length)
{
    if (!_hio_module_rs485._initialized || _hio_module_rs485._read_fifo == NULL || !_hio_module_rs485._async_read_in_progress)
    {
        return 0;
    }

    return hio_fifo_read(_hio_module_rs485._read_fifo, buffer, length);
}

void hio_module_rs485_set_event_handler(void (*event_handler)(hio_module_rs485_event_t, void *), void *event_param)
{
    _hio_module_rs485._event_handler = event_handler;
    _hio_module_rs485._event_param = event_param;
}

size_t hio_module_rs485_write(uint8_t *buffer, size_t length)
{
    return hio_sc16is740_write(&_hio_module_rs485._sc16is750, buffer, length);
}

bool hio_module_rs485_available(size_t *available)
{
    return hio_sc16is740_available(&_hio_module_rs485._sc16is750, available);
}

size_t hio_module_rs485_read(uint8_t *buffer, size_t length, hio_tick_t timeout)
{
    return hio_sc16is740_read(&_hio_module_rs485._sc16is750, buffer, length, timeout);
}

bool hio_module_rs485_set_baudrate(hio_module_rs485_baudrate_t baudrate)
{
    return hio_sc16is740_set_baudrate(&_hio_module_rs485._sc16is750, baudrate);
}

