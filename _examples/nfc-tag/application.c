#include <application.h>

twr_led_t led;

twr_tag_nfc_t tag_nfc;

void application_init(void)
{
    twr_led_init(&led, TWR_GPIO_LED, false, false);

    twr_log_init(TWR_LOG_LEVEL_DEBUG, TWR_LOG_TIMESTAMP_ABS);

    twr_log_info("twr_tag_nfc_init");

    if (twr_tag_nfc_init(&tag_nfc, TWR_I2C_I2C0, TWR_TAG_NFC_I2C_ADDRESS_DEFAULT))
    {
        twr_log_info("ok");
    }
    else
    {
        twr_log_error("error");
    }

    twr_tag_nfc_ndef_t ndef;

    twr_tag_nfc_ndef_init(&ndef);

    twr_tag_nfc_ndef_add_text(&ndef, "HARDWARIO home page", "en");

    twr_tag_nfc_ndef_add_uri(&ndef, "https://www.hardwario.com/");

    twr_tag_nfc_ndef_add_text(&ndef, "Documentation", "en");

    twr_tag_nfc_ndef_add_uri(&ndef, "https://developers.hardwario.com/");

    twr_log_info("twr_tag_nfc_memory_write_ndef");

    if (twr_tag_nfc_memory_write_ndef(&tag_nfc, &ndef))
    {
        twr_log_info("ok");
    }
    else
    {
        twr_log_error("error");
    }

    twr_led_pulse(&led, 2000);
}
