#include <bc_module_sigfox.h>

static void _bc_module_sigfox_event_handler_td1207r(bc_td1207r_t *child, bc_td1207r_event_t event, void *event_param);

static void _bc_module_sigfox_event_handler_wssfm10r1at(bc_wssfm10r1at_t *child, bc_wssfm10r1at_event_t event, void *event_param);

void bc_module_sigfox_init(bc_module_sigfox_t *self, bc_module_sigfox_revision_t revision)
{
    memset(self, 0, sizeof(*self));

    self->_revision = revision;

    if (self->_revision == BC_MODULE_SIGFOX_REVISION_R1)
    {
        bc_td1207r_init(&self->_modem.td1207r, BC_GPIO_P6, BC_UART_UART1);

        bc_td1207r_set_event_handler(&self->_modem.td1207r, _bc_module_sigfox_event_handler_td1207r, self);
    }
    else
    {
        bc_wssfm10r1at_init(&self->_modem.wssfm10r1at, BC_GPIO_P6, BC_UART_UART1);

        bc_wssfm10r1at_set_event_handler(&self->_modem.wssfm10r1at, _bc_module_sigfox_event_handler_wssfm10r1at, self);
    }
}

void bc_module_sigfox_set_event_handler(bc_module_sigfox_t *self, void (*event_handler)(bc_module_sigfox_t *, bc_module_sigfox_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;

    self->_event_param = event_param;
}

bool bc_module_sigfox_is_ready(bc_module_sigfox_t *self)
{
    if (self->_revision == BC_MODULE_SIGFOX_REVISION_R1)
    {
        return bc_td1207r_is_ready(&self->_modem.td1207r);
    }

    return bc_wssfm10r1at_is_ready(&self->_modem.wssfm10r1at);
}

bool bc_module_sigfox_send_rf_frame(bc_module_sigfox_t *self, const void *buffer, size_t length)
{
    if (self->_revision == BC_MODULE_SIGFOX_REVISION_R1)
    {
        return bc_td1207r_send_rf_frame(&self->_modem.td1207r, buffer, length);
    }

    return bc_wssfm10r1at_send_rf_frame(&self->_modem.wssfm10r1at, buffer, length);
}

bool bc_module_sigfox_read_device_id(bc_module_sigfox_t *self)
{
    if (self->_revision == BC_MODULE_SIGFOX_REVISION_R1)
    {
        return true;
    }

    return bc_wssfm10r1at_read_device_id(&self->_modem.wssfm10r1at);
}

bool bc_module_sigfox_read_device_pac(bc_module_sigfox_t *self)
{
    if (self->_revision == BC_MODULE_SIGFOX_REVISION_R1)
    {
        return true;
    }

    return bc_wssfm10r1at_read_device_pac(&self->_modem.wssfm10r1at);
}

bool bc_module_sigfox_continuous_wave(bc_module_sigfox_t *self)
{
    if (self->_revision == BC_MODULE_SIGFOX_REVISION_R1)
    {
        return false;
    }

    return bc_wssfm10r1at_continuous_wave(&self->_modem.wssfm10r1at);
}

static void _bc_module_sigfox_event_handler_td1207r(bc_td1207r_t *child, bc_td1207r_event_t event, void *event_param)
{
    (void) child;

    bc_module_sigfox_t *self = event_param;

    if (self->_event_handler != NULL)
    {
        if (event == BC_TD1207R_EVENT_READY)
        {
            self->_event_handler(self, BC_MODULE_SIGFOX_EVENT_READY, self->_event_param);
        }
        else if (event == BC_TD1207R_EVENT_ERROR)
        {
            self->_event_handler(self, BC_MODULE_SIGFOX_EVENT_ERROR, self->_event_param);
        }
        else if (event == BC_TD1207R_EVENT_SEND_RF_FRAME_START)
        {
            self->_event_handler(self, BC_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_START, self->_event_param);
        }
        else if (event == BC_TD1207R_EVENT_SEND_RF_FRAME_DONE)
        {
            self->_event_handler(self, BC_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_DONE, self->_event_param);
        }
    }
}

static void _bc_module_sigfox_event_handler_wssfm10r1at(bc_wssfm10r1at_t *child, bc_wssfm10r1at_event_t event, void *event_param)
{
    (void) child;

    bc_module_sigfox_t *self = event_param;

    if (self->_event_handler != NULL)
    {
        if (event == BC_WSSFM10R1AT_EVENT_READY)
        {
            self->_event_handler(self, BC_MODULE_SIGFOX_EVENT_READY, self->_event_param);
        }
        else if (event == BC_WSSFM10R1AT_EVENT_ERROR)
        {
            self->_event_handler(self, BC_MODULE_SIGFOX_EVENT_ERROR, self->_event_param);
        }
        else if (event == BC_WSSFM10R1AT_EVENT_SEND_RF_FRAME_START)
        {
            self->_event_handler(self, BC_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_START, self->_event_param);
        }
        else if (event == BC_WSSFM10R1AT_EVENT_SEND_RF_FRAME_DONE)
        {
            self->_event_handler(self, BC_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_DONE, self->_event_param);
        }
        else if (event == BC_WSSFM10R1AT_EVENT_READ_DEVICE_ID)
        {
            self->_event_handler(self, BC_MODULE_SIGFOX_EVENT_READ_DEVICE_ID, self->_event_param);
        }
        else if (event == BC_WSSFM10R1AT_EVENT_READ_DEVICE_PAC)
        {
            self->_event_handler(self, BC_MODULE_SIGFOX_EVENT_READ_DEVICE_PAC, self->_event_param);
        }
    }
}
