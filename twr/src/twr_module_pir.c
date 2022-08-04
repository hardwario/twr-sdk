#include <twr_module_pir.h>

void twr_module_pir_init(twr_module_pir_t *self)
{
    twr_pyq1648_init(self, TWR_GPIO_P8, TWR_GPIO_P9);
}

void twr_module_pir_set_event_handler(twr_module_pir_t *self, void (*event_handler)(twr_module_pir_t *, twr_module_pir_event_t, void*), void *event_param)
{
    twr_pyq1648_set_event_handler(self, (void (*)(twr_pyq1648_t*, twr_pyq1648_event_t, void*)) event_handler, event_param);
}

void twr_module_pir_set_sensitivity(twr_module_pir_t *self, twr_module_pir_sensitivity_t sensitivity)
{
    twr_pyq1648_set_sensitivity(self, (twr_pyq1648_sensitivity_t) sensitivity);
}
