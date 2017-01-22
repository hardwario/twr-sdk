#include <bc_accelerometer.h>
#include <bc_scheduler.h>

#define BC_ACCELEROMETER_DELAY_RUN 50
#define BC_ACCELEROMETER_DELAY_READ 50

static bc_tick_t _bc_accelerometer_task(void *param);

bool bc_accelerometer_init(bc_accelerometer_t *self, bc_i2c_channel_t i2c_channel, uint8_t i2c_address)
{
    uint8_t who_am_i;

	memset(self, 0, sizeof(*self));

    self->_i2c_channel = i2c_channel;
    self->_i2c_address = i2c_address;

	self->_communication_fault = true;

    // Read WHO_AM_I
    if(!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x0F, &who_am_i))
	{
		return false;
	}

    // Check WHO_AM_I register
    if(who_am_i != 0x33)
    {
        return false;
    }

	if (!bc_accelerometer_power_down(self))
	{
		return false;
	}

    if(!bc_accelerometer_continuous_conversion(self))
    {
        return false;
    }

    bc_scheduler_register(_bc_accelerometer_task, self, BC_ACCELEROMETER_DELAY_RUN);

	return true;
}

void bc_accelerometer_set_event_handler(bc_accelerometer_t *self, void (*event_handler)(bc_accelerometer_t *, bc_accelerometer_event_t))
{
    self->_event_handler = event_handler;
}

bool bc_accelerometer_is_communication_fault(bc_accelerometer_t *self)
{
	return self->_communication_fault;
}

bool bc_accelerometer_get_state(bc_accelerometer_t *self, bc_accelerometer_state_t *state)
{
    (void)self;
    (void)state;
    
    /*
	uint8_t value;

	if (!_bc_accelerometer_read_register(self, 0x20, &value))
	{
		return false;
	}

	if ((value & 0xF0) == 0)
	{
		*state = BC_ACCELEROMETER_STATE_POWER_DOWN;
	}

	if (!_bc_accelerometer_read_register(self, 0x27, &value))
	{
		return false;
	}

	if ((value & 0x08) != 0)
	{
		*state = BC_ACCELEROMETER_STATE_RESULT_READY;
	}
	else
	{
		*state = BC_ACCELEROMETER_STATE_CONVERSION;
	}*/

	return true;
}

bool bc_accelerometer_power_down(bc_accelerometer_t *self)
{
	if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x20, 0x07))
	{
		return false;
	}

	return true;
}

bool bc_accelerometer_continuous_conversion(bc_accelerometer_t *self)
{
	if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x23, 0x98))
	{
		return false;
	}

	if (!bc_i2c_write_8b(self->_i2c_channel, self->_i2c_address, 0x20, 0x27))
	{
		return false;
	}

	return true;
}

bool bc_accelerometer_read_result(bc_accelerometer_t *self)
{
    /*
    // Dont work yet, needs I2C repeated start reading
    bc_i2c_tranfer_t transfer;

    transfer.device_address = self->_i2c_address;
    transfer.memory_address = 0x28;
    transfer.buffer = &self->_out_x_l;
    transfer.length = 6;

    return bc_i2c_read(self->_i2c_channel, &transfer);*/


	if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x28, &self->_out_x_l))
	{
		return false;
	}

	if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x29, &self->_out_x_h))
	{
		return false;
	}

	if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x2A, &self->_out_y_l))
	{
		return false;
	}

	if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x2B, &self->_out_y_h))
	{
		return false;
	}

	if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x2C, &self->_out_z_l))
	{
		return false;
	}

	if (!bc_i2c_read_8b(self->_i2c_channel, self->_i2c_address, 0x2D, &self->_out_z_h))
	{
		return false;
	}

	return true;
}

bool bc_accelerometer_get_result_raw(bc_accelerometer_t *self, bc_accelerometer_result_raw_t *result_raw)
{
	result_raw->x_axis = (int16_t) self->_out_x_h;
	result_raw->x_axis <<= 8;
	result_raw->x_axis >>= 4;
	result_raw->x_axis |= ((int16_t) self->_out_x_l) >> 4;

	result_raw->y_axis = (int16_t) self->_out_y_h;
	result_raw->y_axis <<= 8;
	result_raw->y_axis >>= 4;
	result_raw->y_axis |= ((int16_t) self->_out_y_l) >> 4;

	result_raw->z_axis = (int16_t) self->_out_z_h;
	result_raw->z_axis <<= 8;
	result_raw->z_axis >>= 4;
	result_raw->z_axis |= ((int16_t) self->_out_z_l) >> 4;

	return true;
}

bool bc_accelerometer_get_result_g(bc_accelerometer_t *self, bc_accelerometer_result_g_t *result_g)
{
	bc_accelerometer_result_raw_t result_raw;

	if (!bc_accelerometer_get_result_raw(self, &result_raw))
	{
		return false;
	}

	result_g->x_axis = ((float) result_raw.x_axis) / 512.f;
	result_g->y_axis = ((float) result_raw.y_axis) / 512.f;
	result_g->z_axis = ((float) result_raw.z_axis) / 512.f;

	return true;
}


static bc_tick_t _bc_accelerometer_task(void *param)
{
    bc_accelerometer_t *self = param;

start:

    switch (self->_state)
    {
        case BC_ACCELEROMETER_STATE_ERROR:
        {
            self->_accelerometer_valid = false;

            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_ACCELEROMETER_EVENT_ERROR);
            }

            self->_state = BC_ACCELEROMETER_STATE_MEASURE;

            return self->_update_interval;
        }
        case BC_ACCELEROMETER_STATE_MEASURE:
        {
            self->_state = BC_ACCELEROMETER_STATE_ERROR;
/*
            if (!bc_accelerometer_read_result(self))
            {
                goto start;
            }*/

            self->_state = BC_ACCELEROMETER_STATE_READ;

            return BC_ACCELEROMETER_DELAY_READ;
        }
        case BC_ACCELEROMETER_STATE_READ:
        {
            self->_state = BC_ACCELEROMETER_STATE_ERROR;

            if (!bc_accelerometer_read_result(self))
            {
                goto start;
            }

            self->_accelerometer_valid = true;

            self->_state = BC_ACCELEROMETER_STATE_UPDATE;

            goto start;
        }
        case BC_ACCELEROMETER_STATE_UPDATE:
        {
            if (self->_event_handler != NULL)
            {
                self->_event_handler(self, BC_ACCELEROMETER_EVENT_UPDATE);
            }

            self->_state = BC_ACCELEROMETER_STATE_MEASURE;

            return self->_update_interval;
        }
        default:
        {
            self->_state = BC_ACCELEROMETER_STATE_ERROR;

            goto start;
        }
    }
}
