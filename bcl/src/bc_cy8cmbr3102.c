#include <bc_cy8cmbr3102.h>
#include <bc_log.h>

#define _BC_CY8CMBR3102_START_INTERVAL 1500
#define _BC_CY8CMBR3102_SCAN_INTERVAL 100
//#define _BC_CY8CMBR3102_SCAN_INTERVAL_IS_TOUCH 1000

static uint8_t _bc_cy8cmbr3102_default_setting[128] = {

//    x0    x1    x2    x3    x4    x5    x6    x7    x8    x9    xa    xb    xc    xd    xe    xf

    0x01,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0x32, 0x7F,    0,    0,  // 0x
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0x03,    0,    0,    0,  // 1x
       0,    0,    0,    0,    0,    0, 0x01, 0x80, 0x05,    0,    0, 0x02,    0, 0x02,    0,    0,  // 2x
       0,    0,    0,    0,    0, 0x1E,    0,    0,    0, 0x1E,    0,    0,    0,    0,    0,    0,  // 3x
       0, 0xFF,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, 0x03, 0x01, 0x54,  // 4x
       0, 0x37, 0x01,    0,    0, 0x0A,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  // 5x
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,  // 6x
       0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0   // 7x
};

static void _bc_cy8cmbr3102_task(void *param);

static uint16_t _bc_cy8cmbr3102_calculate_crc16(const uint8_t *buffer, uint8_t length);

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
            bc_log_debug("CY8CMBR3102: BC_CY8CMBR3102_STATE_ERROR");

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_CY8CMBR3102_EVENT_ERROR, self->_event_param);
            }

            self->_error_cnt = 0;

            self->_state = BC_CY8CMBR3102_STATE_INITIALIZE;

            bc_scheduler_plan_current_from_now(self->_scan_interval);

            return;
        }
        case BC_CY8CMBR3102_STATE_INITIALIZE:
        {
            bc_log_debug("CY8CMBR3102: BC_CY8CMBR3102_STATE_INITIALIZE");

            bc_i2c_memory_transfer_t transfer;

            transfer.device_address = self->_i2c_address;
            transfer.memory_address = 0;
            transfer.buffer = (uint8_t *)_bc_cy8cmbr3102_default_setting;
            transfer.length = sizeof(_bc_cy8cmbr3102_default_setting);

            uint16_t crc = _bc_cy8cmbr3102_calculate_crc16(_bc_cy8cmbr3102_default_setting, 126);

            _bc_cy8cmbr3102_default_setting[126] = crc;
            _bc_cy8cmbr3102_default_setting[127] = crc >> 8;

            //bc_log_warning("CY8CMBR3102: CRC = %04X", crc);

            if (!bc_i2c_memory_write(self->_i2c_channel, &transfer))
            {
                bc_log_debug("CY8CMBR3102: BC_CY8CMBR3102_STATE_INITIALIZE: failed");

                if (++self->_error_cnt < 3)
                {
                    bc_scheduler_plan_current_from_now(20);

                    return;
                }

                self->_state = BC_CY8CMBR3102_STATE_ERROR;

                goto start;
            }

            self->_error_cnt = 0;

            self->_state = BC_CY8CMBR3102_STATE_CALC_CONFIG_CRC;

            bc_scheduler_plan_current_from_now(50);

            return;
        }
        case BC_CY8CMBR3102_STATE_CALC_CONFIG_CRC:
        {
            bc_log_debug("CY8CMBR3102: BC_CY8CMBR3102_STATE_CALC_CONFIG_CRC");

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
            bc_log_debug("CY8CMBR3102: BC_CY8CMBR3102_STATE_SELF_RESET");

            if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x86, 0xff))
            {
                self->_state = BC_CY8CMBR3102_STATE_ERROR;
                goto start;
            }

            // <<<<<<<<<<<<<<<<<<<<<

            if (!bc_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, 0x82, 0x00))
            {
                self->_state = BC_CY8CMBR3102_STATE_ERROR;
                goto start;
            }

            // <<<<<<<<<<<<<<<<<<<<<

            self->_state = BC_CY8CMBR3102_STATE_READ;

            bc_scheduler_plan_current_from_now(50);

            return;
        }
        case BC_CY8CMBR3102_STATE_READ:
        {
            bc_log_debug("CY8CMBR3102: BC_CY8CMBR3102_STATE_READ");

            uint16_t reg;

            if (!bc_i2c_memory_read_16b(self->_i2c_channel, self->_i2c_address, 0xe2, &reg))
            {
                bc_log_debug("CY8CMBR3102: BC_CY8CMBR3102_STATE_READ: failed");

                if (++self->_error_cnt < 3)
                {
                    bc_scheduler_plan_current_from_now(5);

                    return;
                }

                self->_state = BC_CY8CMBR3102_STATE_ERROR;

                goto start;
            }

            bc_log_info("CY8CMBR3102: DEBUG_RAW_COUNT0 = %04x", reg);


            /*
            bool is_touch;

            if (!bc_cy8cmbr3102_is_touch(self, &is_touch))
            {
                bc_log_debug("CY8CMBR3102: BC_CY8CMBR3102_STATE_READ: failed");

                if (++self->_error_cnt < 3)
                {
                    bc_scheduler_plan_current_from_now(5);

                    return;
                }

                self->_state = BC_CY8CMBR3102_STATE_ERROR;

                goto start;
            }

            self->_error_cnt = 0;

            if (is_touch)
            {
                if (self->_event_handler != NULL)
                {
                    self->_event_handler(self, BC_CY8CMBR3102_EVENT_TOUCH, self->_event_param);
                }

                //bc_scheduler_plan_current_from_now(_BC_CY8CMBR3102_SCAN_INTERVAL_IS_TOUCH);

                //return;
            }
            */

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

static uint16_t _bc_cy8cmbr3102_calculate_crc16(const uint8_t *buffer, uint8_t length)
{
    uint16_t crc16;

    for (crc16 = 0xffff; length != 0; length--, buffer++)
    {
        crc16 ^= *buffer << 8;

        for (int i = 0; i < 8; i++)
        {
            if ((crc16 & 0x8000) != 0)
            {
                crc16 = (crc16 << 1) ^ 0x1021;
            }
            else
            {
                crc16 <<= 1;
            }
        }
    }

    return crc16;
}
