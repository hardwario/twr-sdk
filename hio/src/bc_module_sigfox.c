#include <hio_module_sigfox.h>

static void _hio_module_sigfox_event_handler_td1207r(hio_td1207r_t *child, hio_td1207r_event_t event, void *event_param);

static void _hio_module_sigfox_event_handler_wssfm10r1at(hio_wssfm10r1at_t *child, hio_wssfm10r1at_event_t event, void *event_param);

void hio_module_sigfox_init(hio_module_sigfox_t *self, hio_module_sigfox_revision_t revision)
{
    memset(self, 0, sizeof(*self));

    self->_revision = revision;

    if (self->_revision == HIO_MODULE_SIGFOX_REVISION_R1)
    {
        hio_td1207r_init(&self->_modem.td1207r, HIO_GPIO_P6, HIO_UART_UART1);

        hio_td1207r_set_event_handler(&self->_modem.td1207r, _hio_module_sigfox_event_handler_td1207r, self);
    }
    else
    {
        hio_wssfm10r1at_init(&self->_modem.wssfm10r1at, HIO_GPIO_P6, HIO_UART_UART1);

        hio_wssfm10r1at_set_event_handler(&self->_modem.wssfm10r1at, _hio_module_sigfox_event_handler_wssfm10r1at, self);
    }
}

void hio_module_sigfox_set_event_handler(hio_module_sigfox_t *self, void (*event_handler)(hio_module_sigfox_t *, hio_module_sigfox_event_t, void *), void *event_param)
{
    self->_event_handler = event_handler;

    self->_event_param = event_param;
}

bool hio_module_sigfox_is_ready(hio_module_sigfox_t *self)
{
    if (self->_revision == HIO_MODULE_SIGFOX_REVISION_R1)
    {
        return hio_td1207r_is_ready(&self->_modem.td1207r);
    }

    return hio_wssfm10r1at_is_ready(&self->_modem.wssfm10r1at);
}

bool hio_module_sigfox_send_rf_frame(hio_module_sigfox_t *self, const void *buffer, size_t length)
{
    if (self->_revision == HIO_MODULE_SIGFOX_REVISION_R1)
    {
        return hio_td1207r_send_rf_frame(&self->_modem.td1207r, buffer, length);
    }

    return hio_wssfm10r1at_send_rf_frame(&self->_modem.wssfm10r1at, buffer, length);
}

bool hio_module_sigfox_read_device_id(hio_module_sigfox_t *self)
{
    if (self->_revision == HIO_MODULE_SIGFOX_REVISION_R1)
    {
        return false;
    }

    return hio_wssfm10r1at_read_device_id(&self->_modem.wssfm10r1at);
}

bool hio_module_sigfox_get_device_id(hio_module_sigfox_t *self, char *buffer, size_t buffer_size)
{
    if (self->_revision == HIO_MODULE_SIGFOX_REVISION_R1)
    {
        return false;
    }

    return hio_wssfm10r1at_get_device_id(&self->_modem.wssfm10r1at, buffer, buffer_size);
}

bool hio_module_sigfox_read_device_pac(hio_module_sigfox_t *self)
{
    if (self->_revision == HIO_MODULE_SIGFOX_REVISION_R1)
    {
        return false;
    }

    return hio_wssfm10r1at_read_device_pac(&self->_modem.wssfm10r1at);
}

bool hio_module_sigfox_get_device_pac(hio_module_sigfox_t *self, char *buffer, size_t buffer_size)
{
    if (self->_revision == HIO_MODULE_SIGFOX_REVISION_R1)
    {
        return false;
    }

    return hio_wssfm10r1at_get_device_pac(&self->_modem.wssfm10r1at, buffer, buffer_size);
}

bool hio_module_sigfox_continuous_wave(hio_module_sigfox_t *self)
{
    if (self->_revision == HIO_MODULE_SIGFOX_REVISION_R1)
    {
        return false;
    }

    return hio_wssfm10r1at_continuous_wave(&self->_modem.wssfm10r1at);
}

static void _hio_module_sigfox_event_handler_td1207r(hio_td1207r_t *child, hio_td1207r_event_t event, void *event_param)
{
    (void) child;

    hio_module_sigfox_t *self = event_param;

    if (self->_event_handler != NULL)
    {
        if (event == HIO_TD1207R_EVENT_READY)
        {
            self->_event_handler(self, HIO_MODULE_SIGFOX_EVENT_READY, self->_event_param);
        }
        else if (event == HIO_TD1207R_EVENT_ERROR)
        {
            self->_event_handler(self, HIO_MODULE_SIGFOX_EVENT_ERROR, self->_event_param);
        }
        else if (event == HIO_TD1207R_EVENT_SEND_RF_FRAME_START)
        {
            self->_event_handler(self, HIO_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_START, self->_event_param);
        }
        else if (event == HIO_TD1207R_EVENT_SEND_RF_FRAME_DONE)
        {
            self->_event_handler(self, HIO_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_DONE, self->_event_param);
        }
    }
}

static void _hio_module_sigfox_event_handler_wssfm10r1at(hio_wssfm10r1at_t *child, hio_wssfm10r1at_event_t event, void *event_param)
{
    (void) child;

    hio_module_sigfox_t *self = event_param;

    if (self->_event_handler != NULL)
    {
        if (event == HIO_WSSFM10R1AT_EVENT_READY)
        {
            self->_event_handler(self, HIO_MODULE_SIGFOX_EVENT_READY, self->_event_param);
        }
        else if (event == HIO_WSSFM10R1AT_EVENT_ERROR)
        {
            self->_event_handler(self, HIO_MODULE_SIGFOX_EVENT_ERROR, self->_event_param);
        }
        else if (event == HIO_WSSFM10R1AT_EVENT_SEND_RF_FRAME_START)
        {
            self->_event_handler(self, HIO_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_START, self->_event_param);
        }
        else if (event == HIO_WSSFM10R1AT_EVENT_SEND_RF_FRAME_DONE)
        {
            self->_event_handler(self, HIO_MODULE_SIGFOX_EVENT_SEND_RF_FRAME_DONE, self->_event_param);
        }
        else if (event == HIO_WSSFM10R1AT_EVENT_READ_DEVICE_ID)
        {
            self->_event_handler(self, HIO_MODULE_SIGFOX_EVENT_READ_DEVICE_ID, self->_event_param);
        }
        else if (event == HIO_WSSFM10R1AT_EVENT_READ_DEVICE_PAC)
        {
            self->_event_handler(self, HIO_MODULE_SIGFOX_EVENT_READ_DEVICE_PAC, self->_event_param);
        }
    }
}
