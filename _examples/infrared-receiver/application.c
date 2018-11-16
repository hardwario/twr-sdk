#include <application.h>

// Infrared receive handler
static void ir_event_handler(bc_ir_rx_event_t event, void *param)
{
    (void) param;

    // Received command in NEC format
    if (event == BC_IR_RX_NEC_FORMAT)
	{
        uint32_t nec_code;
        // Get latest command
        bc_ir_rx_get_code(&nec_code);

        // Print command
        bc_log_debug("nec_code: 0x%08x", nec_code);
	}
}

void application_init(void)
{
    bc_log_init(BC_LOG_LEVEL_DUMP, BC_LOG_TIMESTAMP_ABS);

    // Enable PLL for USB and infrared driver
    bc_system_pll_enable();

    // Init infrared driver
    bc_ir_rx_init();
    bc_ir_rx_set_event_handler(ir_event_handler, NULL);
}
