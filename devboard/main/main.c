#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

/* FieldSec Wi-Fi Developer Board companion v0.8
 *
 * Purpose: passive WLAN inventory/teaching telemetry for authorized assessments.
 * This companion does not deauthenticate stations, inject management frames,
 * capture credentials, host credential portals, or attempt authentication.
 *
 * Transport in v0.4 uses the board UART0 console as a receive-only telemetry path
 * to the Flipper USART. The longer-term roadmap also tracks Flipper's official
 * Expansion Module Protocol for smart-module negotiation and RPC integration.
 */

#define FIELDSEC_DEVBOARD_VERSION "1.0.0"
#define MAX_APS 48

static void wifi_init(void) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static const char* auth_name(wifi_auth_mode_t a) {
    switch(a) {
        case WIFI_AUTH_OPEN: return "OPEN";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA";
        case WIFI_AUTH_WPA2_PSK: return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-EAP";
        case WIFI_AUTH_WPA3_PSK: return "WPA3";
        case WIFI_AUTH_WPA2_WPA3_PSK: return "WPA2/WPA3";
        default: return "OTHER";
    }
}

static void print_capabilities(void) {
    printf("FS2|READY|FieldSec-DevBoard|%s\n", FIELDSEC_DEVBOARD_VERSION);
    printf("FS2|CAPS|WIFI_SCAN|PASSIVE_METADATA_ONLY\n");
}

static void scan_once(void) {
    wifi_scan_config_t conf = {0};
    conf.scan_type = WIFI_SCAN_TYPE_PASSIVE;
    conf.scan_time.passive = 120;
    esp_err_t err = esp_wifi_scan_start(&conf, true);
    if(err != ESP_OK) {
        printf("FS2|ERROR|SCAN_START|%d\n", (int)err);
        return;
    }

    uint16_t n = 0;
    err = esp_wifi_scan_get_ap_num(&n);
    if(err != ESP_OK) {
        printf("FS2|ERROR|SCAN_COUNT|%d\n", (int)err);
        return;
    }
    if(n > MAX_APS) n = MAX_APS;

    wifi_ap_record_t* aps = calloc(n ? n : 1, sizeof(wifi_ap_record_t));
    if(!aps) {
        printf("FS2|ERROR|NO_MEMORY|0\n");
        return;
    }

    uint16_t got = n;
    if(n) {
        err = esp_wifi_scan_get_ap_records(&got, aps);
        if(err != ESP_OK) {
            printf("FS2|ERROR|SCAN_RECORDS|%d\n", (int)err);
            free(aps);
            return;
        }
    }

    printf("FS2|SCAN_BEGIN|%u\n", got);
    for(uint16_t i = 0; i < got; i++) {
        char ssid[33];
        memcpy(ssid, aps[i].ssid, 32);
        ssid[32] = '\0';
        for(size_t j = 0; ssid[j]; j++) {
            if(ssid[j] == '|' || ssid[j] == '\r' || ssid[j] == '\n') ssid[j] = '_';
        }
        printf(
            "FS2|AP|%02X:%02X:%02X:%02X:%02X:%02X|%d|%u|%s|%s\n",
            aps[i].bssid[0], aps[i].bssid[1], aps[i].bssid[2],
            aps[i].bssid[3], aps[i].bssid[4], aps[i].bssid[5],
            aps[i].rssi, aps[i].primary, auth_name(aps[i].authmode), ssid);
    }
    printf("FS2|SCAN_END|%u\n", got);
    free(aps);
}

void app_main(void) {
    esp_err_t r = nvs_flash_init();
    if(r == ESP_ERR_NVS_NO_FREE_PAGES || r == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    wifi_init();
    print_capabilities();

    while(true) {
        scan_once();
        vTaskDelay(pdMS_TO_TICKS(15000));
    }
}
