/**
 * @file wifi_ap.h
 * @brief Plik nagłówkowy zawierający funkcje do inicjalizacji trybu Access Point na ESP32.
 */
 
#ifndef WIFI_AP_H
#define WIFI_AP_H

/**
 * @brief Inicjalizuje moduł WiFi w trybie Access Point.
 * 
 * Funkcja konfiguruje ESP32 jako punkt dostępowy (AP), tworząc własną sieć WiFi
 * o nazwie zdefiniowanej w `WIFI_AP_SSID` i haśle `WIFI_AP_PASS`. Umożliwia połączenie
 * maksymalnie `MAX_STA_CONN` klientów. Inicjalizuje stos TCP/IP, domyślny interfejs AP
 * oraz konfigurację WiFi. Uruchomienie AP umożliwia klientom dostęp do usług HTTP serwera.
 * 
 * Jeśli hasło (`WIFI_AP_PASS`) ma długość 0, sieć zostanie utworzona jako otwarta (bez hasła).
 * 
 * @note Wymaga wcześniejszego zainicjalizowania systemu FreeRTOS i wywołania `app_main()`.
 * 
 * @return Brak wartości zwracanej. W przypadku błędu – wywoływane są makra `ESP_ERROR_CHECK()`.
 */
void wifi_init_softap(void);

#endif // WIFI_AP_H