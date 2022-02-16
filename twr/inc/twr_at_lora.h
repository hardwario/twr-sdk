#ifndef _TWR_AT_LORA_H
#define _TWR_AT_LORA_H

#include <twr_common.h>
#include <twr_atci.h>
#include <twr_cmwx1zzabz.h>

#define TWR_AT_LORA_COMMANDS {"$DEVEUI", NULL, twr_at_lora_deveui_set, twr_at_lora_deveui_read, NULL, ""},\
                         {"$DEVADDR", NULL, twr_at_lora_devaddr_set, twr_at_lora_devaddr_read, NULL, ""},\
                         {"$NWKSKEY", NULL, twr_at_lora_nwkskey_set, twr_at_lora_nwkskey_read, NULL, ""},\
                         {"$APPSKEY", NULL, twr_at_lora_appskey_set, twr_at_lora_appskey_read, NULL, ""},\
                         {"$APPKEY", NULL, twr_at_lora_appkey_set, twr_at_lora_appkey_read, NULL, ""},\
                         {"$APPEUI", NULL, twr_at_lora_appeui_set, twr_at_lora_appeui_read, NULL, ""},\
                         {"$BAND", NULL, twr_at_lora_band_set, twr_at_lora_band_read, NULL, "0:AS923, 1:AU915, 5:EU868, 6:KR920, 7:IN865, 8:US915"},\
                         {"$MODE", NULL, twr_at_lora_mode_set, twr_at_lora_mode_read, NULL, "0:ABP, 1:OTAA"},\
                         {"$NWK", NULL, twr_at_lora_nwk_set, twr_at_lora_nwk_read, NULL, "Network type 0:private, 1:public"},\
                         {"$ADR", NULL, twr_at_lora_adr_set, twr_at_lora_adr_read, NULL, "Automatic data rate 0:disabled, 1:enabled"},\
                         {"$DR", NULL, twr_at_lora_dr_set, twr_at_lora_dr_read, NULL, "Data rate 0-15"},\
                         {"$REPU", NULL, twr_at_lora_repu_set, twr_at_lora_repu_read, NULL, "Repeat of unconfirmed transmissions 1-15"},\
                         {"$REPC", NULL, twr_at_lora_repc_set, twr_at_lora_repc_read, NULL, "Repeat of confirmed transmissions 1-8"},\
                         {"$JOIN", twr_at_lora_join, NULL, NULL, NULL, "Send OTAA Join packet"},\
                         {"$FRMCNT", twr_at_lora_frmcnt, NULL, NULL, NULL, "Get frame counters"},\
                         {"$LNCHECK", twr_at_lora_link_check, NULL, NULL, NULL, "MAC Link Check"},\
                         {"$RFQ", twr_at_lora_rfq, NULL, NULL, NULL, "Get RSSI/SNR of last RX packet"},\
                         {"$AT", NULL, twr_at_lora_custom_at_set, NULL, NULL, "Send custom AT command"},\
                         {"$DEBUG", NULL, twr_at_lora_debug_set, NULL, NULL, "Show debug UART communication"},\
                         {"$REBOOT", twr_at_lora_reboot, NULL, NULL, NULL, "Firmware reboot"},\
                         {"$FRESET", twr_at_lora_freset, NULL, NULL, NULL, "LoRa Module factory reset"},\
                         {"$FWVER", twr_at_lora_ver_read, NULL, NULL, NULL, "Show LoRa Module firmware version"}

void twr_at_lora_init(twr_cmwx1zzabz_t *lora);

bool twr_at_lora_deveui_read(void);
bool twr_at_lora_deveui_set(twr_atci_param_t *param);

bool twr_at_lora_devaddr_read(void);
bool twr_at_lora_devaddr_set(twr_atci_param_t *param);

bool twr_at_lora_nwkskey_read(void);
bool twr_at_lora_nwkskey_set(twr_atci_param_t *param);

bool twr_at_lora_appskey_read(void);
bool twr_at_lora_appskey_set(twr_atci_param_t *param);

bool twr_at_lora_appkey_read(void);
bool twr_at_lora_appkey_set(twr_atci_param_t *param);

bool twr_at_lora_appeui_read(void);
bool twr_at_lora_appeui_set(twr_atci_param_t *param);

bool twr_at_lora_band_read(void);
bool twr_at_lora_band_set(twr_atci_param_t *param);

bool twr_at_lora_mode_read(void);
bool twr_at_lora_mode_set(twr_atci_param_t *param);

bool twr_at_lora_nwk_read(void);
bool twr_at_lora_nwk_set(twr_atci_param_t *param);

bool twr_at_lora_adr_read(void);
bool twr_at_lora_adr_set(twr_atci_param_t *param);

bool twr_at_lora_dr_read(void);
bool twr_at_lora_dr_set(twr_atci_param_t *param);

bool twr_at_lora_repu_read(void);
bool twr_at_lora_repu_set(twr_atci_param_t *param);

bool twr_at_lora_repc_read(void);
bool twr_at_lora_repc_set(twr_atci_param_t *param);

bool twr_at_lora_ver_read(void);

bool twr_at_lora_reboot(void);
bool twr_at_lora_freset(void);
bool twr_at_lora_frmcnt(void);
bool twr_at_lora_link_check(void);
bool twr_at_lora_rfq(void);
bool twr_at_lora_custom_at_set(twr_atci_param_t *param);

bool twr_at_lora_join(void);
bool twr_at_lora_debug_set(twr_atci_param_t *param);

#endif
