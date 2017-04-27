#include <bc_module_pir.h>

void bc_module_pir_init(bc_module_pir_t *self)
{
    bc_pyq1648_init(self, BC_GPIO_P8, BC_GPIO_P9);
}

void bc_module_pir_set_event_handler(bc_module_pir_t *self, void (*event_handler)(bc_module_pir_t *, bc_module_pir_event_t, void*), void *event_param)
{
    bc_pyq1648_set_event_handler(self, (void (*)(bc_pyq1648_t*, bc_pyq1648_event_t, void*)) event_handler, event_param);
}

void bc_module_pir_set_sensitivity(bc_module_pir_t *self, bc_module_pir_sensitivity_t sensitivity)
{
    bc_pyq1648_set_sensitivity(self, sensitivity);
}
