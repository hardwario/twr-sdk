#include <twr_hts221.h>

#define HTS221_WHO_AM_I 0x0F
#define HTS221_WHO_AM_I_RESULT 0xBC
#define HTS221_AV_CONF 0x10
#define HTS221_CTRL_REG1 0x20
#define HTS221_CTRL_REG2 0x21
#define HTS221_CTRL_REG3 0x22
#define HTS221_STATUS_REG 0x27
#define HTS221_HUMIDITY_OUT_L 0x28
#define HTS221_HUMIDITY_OUT_H 0x29
#define HTS221_TEMP_OUT_L 0x2A
#define HTS221_TEMP_OUT_H 0x2B
#define HTS221_CALIB_OFFSET 0x30
#define HTS221_CALIB_0 0x30
#define HTS221_CALIB_1 0x31
#define HTS221_CALIB_2 0x32
#define HTS221_CALIB_3 0x33
#define HTS221_CALIB_4 0x34
#define HTS221_CALIB_5 0x35
#define HTS221_CALIB_6 0x36
#define HTS221_CALIB_7 0x37
#define HTS221_CALIB_8 0x38
#define HTS221_CALIB_9 0x39
#define HTS221_CALIB_A 0x3A
#define HTS221_CALIB_B 0x3B
#define HTS221_CALIB_C 0x3C
#define HTS221_CALIB_D 0x3D
#define HTS221_CALIB_E 0x3E
#define HTS221_CALIB_F 0x3F
#define HTS221_BIT_PD 0x80
#define HTS221_BIT_BDU 0x04
#define HTS221_BIT_ONE_SHOT 0x01
#define HTS221_BIT_T_DA 0x01
#define HTS221_BIT_H_DA 0x02
#define HTS221_MASK_ODR 0x03
#define HTS221_ODR_ONE_SHOT 0x00
#define HTS221_ODR_1_HZ 0x01
#define HTS221_ODR_7_HZ 0x02
#define HTS221_ODR_12_HZ 0x03

// TODO Clarify timing with ST
#define _TWR_HTS221_DELAY_RUN 50
#define _TWR_HTS221_DELAY_INITIALIZATION 50
#define _TWR_HTS221_DELAY_MEASUREMENT 50

static void _twr_hts221_task_interval(void *param);

static void _twr_hts221_task_measure(void *param);

static bool _twr_hts221_load_calibration(twr_hts221_t *self);

void twr_hts221_init(twr_hts221_t *self, twr_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

    self->_task_id_interval = twr_scheduler_register(_twr_hts221_task_interval, self, TWR_TICK_INFINITY);
    self->_task_id_measure = twr_scheduler_register(_twr_hts221_task_measure, self, _TWR_HTS221_DELAY_RUN);

    self->_tick_ready = _TWR_HTS221_DELAY_RUN;

    twr_i2c_init(self->_i2c_channel, TWR_I2C_SPEED_400_KHZ);

    // TODO This delays initialization, should be part of state machine
    _twr_hts221_load_calibration(self);
}

void twr_hts221_deinit(twr_hts221_t *self)
{
    uint8_t ctrl_reg1;
    if (!twr_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, HTS221_CTRL_REG1, &ctrl_reg1))
    {
        ctrl_reg1 &= ~HTS221_BIT_PD;
        twr_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, HTS221_CTRL_REG1, ctrl_reg1);
    }

    twr_scheduler_unregister(self->_task_id_interval);
    twr_scheduler_unregister(self->_task_id_measure);
}

void twr_hts221_set_event_handler(twr_hts221_t *self, void (*event_handler)(twr_hts221_t *, twr_hts221_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;
    self->_event_param = event_param;
}

void twr_hts221_set_update_interval(twr_hts221_t *self, twr_tick_t interval)
{
    self->_update_interval = interval;

    if (self->_update_interval == TWR_TICK_INFINITY)
    {
        twr_scheduler_plan_absolute(self->_task_id_interval, TWR_TICK_INFINITY);
    }
    else
    {
        twr_scheduler_plan_relative(self->_task_id_interval, self->_update_interval);

        twr_hts221_measure(self);
    }
}

bool twr_hts221_measure(twr_hts221_t *self)
{
    if (self->_measurement_active)
    {
        return false;
    }

    self->_measurement_active = true;

    twr_scheduler_plan_absolute(self->_task_id_measure, self->_tick_ready);

    return true;
}

bool twr_hts221_get_humidity_percentage(twr_hts221_t *self, float *percentage)
{
    if (!self->_humidity_valid)
    {
        return false;
    }

    *percentage = self->_h0_rh + ((self->_reg_humidity - self->_h0_t0_out) * self->_h_grad);

    if (*percentage >= 100.f)
    {
        *percentage = 100.f;
    }

    return true;
}

static void _twr_hts221_task_interval(void *param)
{
    twr_hts221_t *self = param;

    twr_hts221_measure(self);

    twr_scheduler_plan_current_relative(self->_update_interval);
}

static void _twr_hts221_task_measure(void *param)
{
    twr_hts221_t *self = param;

start:

    switch (self->_state)
    {
        case TWR_HTS221_STATE_ERROR:
        {
            self->_humidity_valid = false;

            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, TWR_HTS221_EVENT_ERROR, self->_event_param);
            }

            self->_state = TWR_HTS221_STATE_INITIALIZE;

            return;
        }
        case TWR_HTS221_STATE_INITIALIZE:
        {
            self->_state = TWR_HTS221_STATE_ERROR;

            uint8_t ctrl_reg1;

            if (!twr_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, HTS221_CTRL_REG1, &ctrl_reg1))
            {
                goto start;
            }

            ctrl_reg1 &= ~HTS221_BIT_PD;

            if (!twr_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, HTS221_CTRL_REG1, ctrl_reg1))
            {
                goto start;
            }

            self->_state = TWR_HTS221_STATE_MEASURE;

            self->_tick_ready = twr_tick_get() + _TWR_HTS221_DELAY_INITIALIZATION;

            if (self->_measurement_active)
            {
                twr_scheduler_plan_current_absolute(self->_tick_ready);
            }

            return;
        }
        case TWR_HTS221_STATE_MEASURE:
        {
            self->_state = TWR_HTS221_STATE_ERROR;

            uint8_t ctrl_reg1;

            if (!twr_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, HTS221_CTRL_REG1, &ctrl_reg1))
            {
                goto start;
            }

            ctrl_reg1 |= HTS221_BIT_PD;

            if (!twr_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, HTS221_CTRL_REG1, ctrl_reg1))
            {
                goto start;
            }

            if (!twr_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, HTS221_CTRL_REG1, HTS221_BIT_PD | HTS221_BIT_BDU))
            {
                goto start;
            }

            if (!twr_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, HTS221_CTRL_REG2, HTS221_BIT_ONE_SHOT))
            {
                goto start;
            }

            self->_state = TWR_HTS221_STATE_READ;

            twr_scheduler_plan_current_from_now(_TWR_HTS221_DELAY_MEASUREMENT);

            return;
        }
        case TWR_HTS221_STATE_READ:
        {
            self->_state = TWR_HTS221_STATE_ERROR;

            uint8_t reg_status;
            uint8_t retval[2];
            uint8_t ctrl_reg1;

            if (!twr_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, HTS221_STATUS_REG, &reg_status))
            {
                goto start;
            }

            if ((reg_status & HTS221_BIT_H_DA) == 0)
            {
                goto start;
            }

            if (!twr_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, HTS221_HUMIDITY_OUT_H, &retval[1]))
            {
                goto start;
            }

            if (!twr_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, HTS221_HUMIDITY_OUT_L, &retval[0]))
            {
                goto start;
            }

            self->_reg_humidity = ((uint16_t) retval[1] << 8) | retval[0];

            if (!twr_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, HTS221_CTRL_REG1, &ctrl_reg1))
            {
                goto start;
            }

            ctrl_reg1 &= ~HTS221_BIT_PD;

            if (!twr_i2c_memory_write_8b(self->_i2c_channel, self->_i2c_address, HTS221_CTRL_REG1, ctrl_reg1))
            {
                goto start;
            }

            self->_humidity_valid = true;

            self->_state = TWR_HTS221_STATE_UPDATE;

            goto start;
        }
        case TWR_HTS221_STATE_UPDATE:
        {
            self->_measurement_active = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, TWR_HTS221_EVENT_UPDATE, self->_event_param);
            }

            self->_state = TWR_HTS221_STATE_MEASURE;

            return;
        }
        default:
        {
            self->_state = TWR_HTS221_STATE_ERROR;

            goto start;
        }
    }
}

static bool _twr_hts221_load_calibration(twr_hts221_t *self)
{
    uint8_t i;
    uint8_t calibration[16];
    int16_t h1_rh;
    int16_t h1_t0_out;

    for (i = 0; i < 16; i++)
    {
        if (!twr_i2c_memory_read_8b(self->_i2c_channel, self->_i2c_address, HTS221_CALIB_OFFSET + i, &calibration[i]))
        {
            return false;
        }
    }

    self->_h0_rh = (int16_t) calibration[0];
    self->_h0_rh >>= 1;
    h1_rh = (int16_t) calibration[1];
    h1_rh >>= 1;

    self->_h0_t0_out = (int16_t) calibration[6];
    self->_h0_t0_out |= ((int16_t) calibration[7]) << 8;

    h1_t0_out = (int16_t) calibration[10];
    h1_t0_out |= ((int16_t) calibration[11]) << 8;

    if ((h1_t0_out - self->_h0_t0_out) == 0)
    {
        return false;
    }

    self->_h_grad = (float) (h1_rh - self->_h0_rh) / (float) (h1_t0_out - self->_h0_t0_out);

    uint16_t t0_degC = (int16_t) calibration[2];
    t0_degC |= (int16_t) (0x03 & calibration[5]) << 8;
    t0_degC >>= 3; // /= 8.0

    uint16_t t1_degC = (int16_t) calibration[3];
    t1_degC |= (int16_t) (0x0C & calibration[5]) << 6;
    t1_degC >>= 3;

    return true;
}
