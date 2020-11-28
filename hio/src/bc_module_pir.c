#include <hio_module_pir.h>

void hio_module_pir_init(hio_module_pir_t *self)
{
    hio_pyq1648_init(self, HIO_GPIO_P8, HIO_GPIO_P9);
}

void hio_module_pir_set_event_handler(hio_module_pir_t *self, void (*event_handler)(hio_module_pir_t *, hio_module_pir_event_t, void*), void *event_param)
{
    hio_pyq1648_set_event_handler(self, (void (*)(hio_pyq1648_t*, hio_pyq1648_event_t, void*)) event_handler, event_param);
}

void hio_module_pir_set_sensitivity(hio_module_pir_t *self, hio_module_pir_sensitivity_t sensitivity)
{
    hio_pyq1648_set_sensitivity(self, sensitivity);
}
