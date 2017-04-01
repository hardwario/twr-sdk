#include <application.h>
#include <usb_talk.h>

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
        // Publish command over USB
        usb_talk_publish_ir_rx("", &nec_code);
	}
}

void application_init(void)
{
    // Enable PLL for USB and infrared driver
    bc_module_core_pll_enable();

    // Init infrared driver
    bc_ir_rx_init();
    bc_ir_rx_set_event_handler(ir_event_handler, NULL);

    // Init USB
    usb_talk_init();
}
