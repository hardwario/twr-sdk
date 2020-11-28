#include <bc_cy8cmbr3102.h>

#define _BC_CY8CMBR3102_START_INTERVAL 1500
#define _BC_CY8CMBR3102_SCAN_INTERVAL 100
#define _BC_CY8CMBR3102_SCAN_INTERVAL_IS_TOUCH 1000

static const uint8_t _bc_cy8cmbr3102_default_setting[] = {
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

static void _bc_cy8cmbr3102_task(void *param);

bool bc_cy8cmbr3102_init(bc_cy8cmbr3102_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    bc_i2c_init(self->_i2c_channel, BC_I2C_SPEED_400_KHZ);

    self->_scan_interval = _BC_CY8CMBR3102_SCAN_INTERVAL;

    self->_task_id_task = bc_scheduler_register(_bc_cy8cmbr3102_task, self, _BC_CY8CMBR3102_START_INTERVAL);

    return true;
}

void bc_cy8cmbr3102_set_event_handler(bc_cy8cmbr3102_t *self, void (*event_handler)(bc_cy8cmbr3102_t *, bc_cy8cmbr3102_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void bc_cy8cmbr3102_set_scan_interval(bc_cy8cmbr3102_t *self, bc_tick_t scan_interval)
{
    self->_scan_interval = scan_interval;

    bc_scheduler_plan_absolute(self->_task_id_task, _BC_CY8CMBR3102_START_INTERVAL < bc_tick_get() ? _BC_CY8CMBR3102_START_INTERVAL: 0);
}

bool bc_cy8cmbr3102_get_proximity(bc_cy8cmbr3102_t *self, uint16_t value)
{
    return bc_i2c_memory_read_16b(self->_i2c_channel, self->_i2c_address, 0xba, &value);
}

bool bc_cy8cmbr3102_is_touch(bc_cy8cmbr3102_t *self, bool *is_touch)
{
    uint8_t prox_stat = 0;

    if (!bc_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, 0xae, &prox_stat))
    {
        return false;
    }

    *is_touch = (prox_stat & 0x01) != 0;

    return true;
}


static void _bc_cy8cmbr3102_task(void *param)
{
    bc_cy8cmbr3102_t *self = param;

    start:

    switch (self->_state)
    {
        case BC_CY8CMBR3102_STATE_ERROR:
        {
            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_CY8CMBR3102_EVENT_ERROR, self->_event_param);
            }

            self->_state = BC_CY8CMBR3102_STATE_INITIALIZE;

            bc_scheduler_plan_current_from_now(self->_scan_interval);

            return;
        }
        case BC_CY8CMBR3102_STATE_INITIALIZE:
        {
            bc_i2c_memory_transfer_t transfer;
            transfer.device_address = self->_i2c_address;
            transfer.memory_address = 0x00;
            transfer.buffer = (uint8_t *)_bc_cy8cmbr3102_default_setting;
            transfer.length = sizeof(_bc_cy8cmbr3102_default_setting);

            if (!bc_i2c_memory_write(self->_i2c_channel, &transfer))
            {
                if (self->_error_cnt++ < 2)
                {
                    bc_scheduler_plan_current_from_now(50);

                    return;
                }

                self->_error_cnt = 0;

                self->_state = BC_CY8CMBR3102_STATE_ERROR;

                goto start;
            }

            self->_state = BC_CY8CMBR3102_STATE_CALC_CONFIG_CRC;

            bc_scheduler_plan_current_from_now(50);

            return;
        }
        case BC_CY8CMBR3102_STATE_CALC_CONFIG_CRC:
        {
            if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x86, 0x02))
            {
                self->_state = BC_CY8CMBR3102_STATE_ERROR;

                goto start;
            }

            self->_state = BC_CY8CMBR3102_STATE_SELF_RESET;

            bc_scheduler_plan_current_from_now(220 + 10);

            return;
        }
        case BC_CY8CMBR3102_STATE_SELF_RESET:
        {
            if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x86, 0xff))
            {
                self->_state = BC_CY8CMBR3102_STATE_ERROR;
                goto start;
            }

            self->_state = BC_CY8CMBR3102_STATE_READ;

            bc_scheduler_plan_current_from_now(50);

            return;
        }
        case BC_CY8CMBR3102_STATE_READ:
        {
            bool is_touch;

            if (!bc_cy8cmbr3102_is_touch(self, &is_touch))
            {
                if (self->_error_cnt++ < 2)
                {
                    bc_scheduler_plan_current_from_now(5);

                    return;
                }

                self->_error_cnt = 0;

                self->_state = BC_CY8CMBR3102_STATE_ERROR;

                goto start;
            }

            if (is_touch)
            {
                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_CY8CMBR3102_EVENT_TOUCH, self->_event_param);
                }

                bc_scheduler_plan_current_from_now(_BC_CY8CMBR3102_SCAN_INTERVAL_IS_TOUCH);

                return;
            }

            bc_scheduler_plan_current_from_now(self->_scan_interval);

            return;
        }
        default:
        {
            self->_state = BC_CY8CMBR3102_STATE_ERROR;

            goto start;
        }
    }
}

