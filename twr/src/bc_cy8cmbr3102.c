#include <twr_cy8cmbr3102.h>

#define _TWR_CY8CMBR3102_START_INTERVAL 1500
#define _TWR_CY8CMBR3102_SCAN_INTERVAL 100
#define _TWR_CY8CMBR3102_SCAN_INTERVAL_IS_TOUCH 1000

static const uint8_t _twr_cy8cmbr3102_default_setting[] = {
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x32, 0x7F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80,
    0x05, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00,
    0x00, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x01, 0x54,
    0x00, 0x37, 0x01, 0x00, 0x00, 0x0A, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6C, 0x0E
};

static void _twr_cy8cmbr3102_task(void *param);

bool twr_cy8cmbr3102_init(twr_cy8cmbr3102_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    twr_i2c_init(self->_i2c_channel, TWR_I2C_SPEED_400_KHZ);

    self->_scan_interval = _TWR_CY8CMBR3102_SCAN_INTERVAL;

    self->_task_id_task = twr_scheduler_register(_twr_cy8cmbr3102_task, self, _TWR_CY8CMBR3102_START_INTERVAL);

    return true;
}

void twr_cy8cmbr3102_set_event_handler(twr_cy8cmbr3102_t *self, void (*event_handler)(twr_cy8cmbr3102_t *, twr_cy8cmbr3102_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void twr_cy8cmbr3102_set_scan_interval(twr_cy8cmbr3102_t *self, twr_tick_t scan_interval)
{
    self->_scan_interval = scan_interval;

    twr_scheduler_plan_absolute(self->_task_id_task, _TWR_CY8CMBR3102_START_INTERVAL < twr_tick_get() ? _TWR_CY8CMBR3102_START_INTERVAL: 0);
}

bool twr_cy8cmbr3102_get_proximity(twr_cy8cmbr3102_t *self, uint16_t value)
{
    return twr_i2c_memory_read_16b(self->_i2c_channel, self->_i2c_address, 0xba, &value);
}

bool twr_cy8cmbr3102_is_touch(twr_cy8cmbr3102_t *self, bool *is_touch)
{
    uint8_t prox_stat = 0;

    if (!twr_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0xae, &prox_stat))
    {
        return false;
    }

    *is_touch = (prox_stat & 0x01) != 0;

    return true;
}


static void _twr_cy8cmbr3102_task(void *param)
{
    twr_cy8cmbr3102_t *self = param;

    start:

    switch (self->_state)
    {
        case TWR_CY8CMBR3102_STATE_ERROR:
        {
            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, TWR_CY8CMBR3102_EVENT_ERROR, self->_event_param);
            }

            self->_state = TWR_CY8CMBR3102_STATE_INITIALIZE;

            twr_scheduler_plan_current_from_now(self->_scan_interval);

            return;
        }
        case TWR_CY8CMBR3102_STATE_INITIALIZE:
        {
            twr_i2c_memory_transfer_t transfer;
            transfer.device_address = self->_i2c_address;
            transfer.memory_address = 0x00;
            transfer.buffer = (uint8_t *)_twr_cy8cmbr3102_default_setting;
            transfer.length = sizeof(_twr_cy8cmbr3102_default_setting);

            if (!twr_i2c_memory_write(self->_i2c_channel, &transfer))
            {
                if (self->_error_cnt++ < 2)
                {
                    twr_scheduler_plan_current_from_now(50);

                    return;
                }

                self->_error_cnt = 0;

                self->_state = TWR_CY8CMBR3102_STATE_ERROR;

                goto start;
            }

            self->_state = TWR_CY8CMBR3102_STATE_CALC_CONFIG_CRC;

            twr_scheduler_plan_current_from_now(50);

            return;
        }
        case TWR_CY8CMBR3102_STATE_CALC_CONFIG_CRC:
        {
            if (!twr_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x86, 0x02))
            {
                self->_state = TWR_CY8CMBR3102_STATE_ERROR;

                goto start;
            }

            self->_state = TWR_CY8CMBR3102_STATE_SELF_RESET;

            twr_scheduler_plan_current_from_now(220 + 10);

            return;
        }
        case TWR_CY8CMBR3102_STATE_SELF_RESET:
        {
            if (!twr_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x86, 0xff))
            {
                self->_state = TWR_CY8CMBR3102_STATE_ERROR;
                goto start;
            }

            self->_state = TWR_CY8CMBR3102_STATE_READ;

            twr_scheduler_plan_current_from_now(50);

            return;
        }
        case TWR_CY8CMBR3102_STATE_READ:
        {
            bool is_touch;

            if (!twr_cy8cmbr3102_is_touch(self, &is_touch))
            {
                if (self->_error_cnt++ < 2)
                {
                    twr_scheduler_plan_current_from_now(5);

                    return;
                }

                self->_error_cnt = 0;

                self->_state = TWR_CY8CMBR3102_STATE_ERROR;

                goto start;
            }

            if (is_touch)
            {
                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, TWR_CY8CMBR3102_EVENT_TOUCH, self->_event_param);
                }

                twr_scheduler_plan_current_from_now(_TWR_CY8CMBR3102_SCAN_INTERVAL_IS_TOUCH);

                return;
            }

            twr_scheduler_plan_current_from_now(self->_scan_interval);

            return;
        }
        default:
        {
            self->_state = TWR_CY8CMBR3102_STATE_ERROR;

            goto start;
        }
    }
}

