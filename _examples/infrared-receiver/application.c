#include <application.h>

// Infrared receive handler
static void ir_event_handler(hio_ir_rx_event_t event, void *param)
{
    (void) param;

    // Received command in NEC format
    if (event == HIO_IR_RX_NEC_FORMAT)
	{
        uint32_t nec_code;
        // Get latest command
        hio_ir_rx_get_code(&nec_code);

        // Print command
        hio_log_debug("nec_code: 0x%08x", nec_code);
	}
}

void application_init(void)
{
    hio_log_init(HIO_LOG_LEVEL_DUMP, HIO_LOG_TIMESTAMP_ABS);

    // Enable PLL for USB and infrared driver
    hio_system_pll_enable();

    // Init infrared driver
    hio_ir_rx_init();
    hio_ir_rx_set_event_handler(ir_event_handler, NULL);
}
