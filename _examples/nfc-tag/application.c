#include <application.h>

hio_led_t led;

hio_tag_nfc_t tag_nfc;

void application_init(void)
{
    hio_led_init(&led, HIO_GPIO_LED, false, false);

    hio_log_init(HIO_LOG_LEVEL_DEBUG, HIO_LOG_TIMESTAMP_ABS);

    hio_log_info("hio_tag_nfc_init");

    if (hio_tag_nfc_init(&tag_nfc, HIO_I2C_I2C0, HIO_TAG_NFC_I2C_ADDRESS_DEFAULT))
    {
        hio_log_info("ok");
    }
    else
    {
        hio_log_error("error");
    }

    hio_tag_nfc_ndef_t ndef;

    hio_tag_nfc_ndef_init(&ndef);

    hio_tag_nfc_ndef_add_text(&ndef, "HARDWARIO home page", "en");

    hio_tag_nfc_ndef_add_uri(&ndef, "https://www.hardwario.com/");

    hio_tag_nfc_ndef_add_text(&ndef, "Documentation", "en");

    hio_tag_nfc_ndef_add_uri(&ndef, "https://developers.hardwario.com/");

    hio_log_info("hio_tag_nfc_memory_write_ndef");

    if (hio_tag_nfc_memory_write_ndef(&tag_nfc, &ndef))
    {
        hio_log_info("ok");
    }
    else
    {
        hio_log_error("error");
    }

    hio_led_pulse(&led, 2000);
}
