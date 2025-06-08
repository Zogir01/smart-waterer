#include "../inc/wifi_ap.h"
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include <string.h>

#define WIFI_AP_SSID      "Podlewajka"
#define WIFI_AP_PASS      "12345678"
#define MAX_STA_CONN   4

static const char *TAG = "wifi_ap";

void wifi_init_softap(void) 
{    
    // Inicjalizacja stosu TCP/IP
    ESP_ERROR_CHECK(esp_netif_init());
    
    // Wymagane przez app_main (jakąś podstawową pętle programu)
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Inicjalizacja domyslnego access-pointa
    esp_netif_create_default_wifi_ap();
    
    // Inicjalizacja domyslnej konfiguracji
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	// Konfiguracja wifi
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .password = WIFI_AP_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

    if (strlen(WIFI_AP_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

	// Ustawienie trybu wifi na access-point
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    
    // Inicjalizacja konfiguracji wifi
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    
    // Wystartowanie wifi
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi AP started. SSID:%s password:%s", WIFI_AP_SSID, WIFI_AP_PASS);
}
