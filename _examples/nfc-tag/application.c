#include <application.h>

bc_led_t led;

bc_tag_nfc_t tag_nfc;

void application_init(void)
{
    bc_led_init(&led, BC_GPIO_LED, false, false);

    bc_log_init(BC_LOG_LEVEL_DEBUG, BC_LOG_TIMESTAMP_ABS);

    bc_log_info("bc_tag_nfc_init");

    if (bc_tag_nfc_init(&tag_nfc, BC_I2C_I2C0, BC_TAG_NFC_I2C_ADDRESS_DEFAULT))
    {
        bc_log_info("ok");
    }
    else
    {
        bc_log_error("error");
    }

    bc_tag_nfc_ndef_t ndef;

    bc_tag_nfc_ndef_init(&ndef);

    bc_tag_nfc_ndef_add_text(&ndef, "BigClown home page", "en");

    bc_tag_nfc_ndef_add_uri(&ndef, "https://www.bigclown.com/");

    bc_tag_nfc_ndef_add_text(&ndef, "Documentation", "en");

    bc_tag_nfc_ndef_add_uri(&ndef, "https://doc.bigclown.com/");

    bc_log_info("bc_tag_nfc_memory_write_ndef");

    if (bc_tag_nfc_memory_write_ndef(&tag_nfc, &ndef))
    {
        bc_log_info("ok");
    }
    else
    {
        bc_log_error("error");
    }

    bc_led_pulse(&led, 2000);
}
