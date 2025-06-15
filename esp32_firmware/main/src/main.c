/**
 * @file main.c
 * @brief Główna logika aplikacji sterującej automatycznym podlewaniem roślin przy użyciu ESP32.
 * 
 * Ten plik zawiera funkcję `app_main()` będącą punktem wejścia programu, a także inicjalizację
 * wszystkich kluczowych komponentów systemu: czujników, kolejek, Wi-Fi, przerwań oraz tasków FreeRTOS.
 * System realizuje automatyczne oraz ręczne podlewanie w zależności od poziomu wilgotności gleby
 * oraz sygnałów wejściowych (przycisk, API).
 *
 * ### Główne komponenty:
 * - **ADC + czujnik wilgotności**: Cyckliczny odczyt wilgotności gleby i konwersja na wartość procentową.
 * - **Pompa wody**: Aktywowana, gdy poziom wilgotności spada poniżej zadanego progu.
 * - **FreeRTOS Taski**:
 *    - `taskReadHumiditySensor`: Odczyt i analiza wilgotności.
 *    - `taskWaterPump`: Obsługa włączania pompy na określony czas.
 *    - `taskHttpServer`: Integracja z API (placeholder).
 * - **Kolejki FreeRTOS**:
 *    - `watering_queue`: Sterowanie włączaniem pompy.
 *    - `humidity_queue`: Przechowywanie aktualnej wilgotności.
 * - **Konfiguracja użytkownika (`user_config`)**: Parametry ustawiane przez użytkownika (czas podlewania, próg wilgotności itd.)
 * 
 * ### Obsługiwane tryby podlewania:
 * - **Automatyczny**: na podstawie odczytu wilgotności.
 * - **Ręczny**: fizyczny przycisk lub żądanie HTTP.
 */

// ------------------------------- STANDARD C ----------------------------------------------

#include <stdio.h>
#include <string.h>
#include <limits.h>

// ------------------------------- ESP-IDF -------------------------------------------------

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"

// ------------------------------- WŁASNE MODUŁY -------------------------------------------

#include "../inc/config.h"
#include "../inc/io_util.h"
#include "../inc/display.h"
#include "../inc/user_config.h"
#include "../inc/http_server.h"
#include "../inc/wifi_ap.h"

// ------------------------------- ZMIENNE GLOBALNE ----------------------------------------

/**
 * @brief TAG do logów dla main.c
 */
static const char *TAG = "MAIN"; // TAG błędów dla main.c

/**
 * @brief Flaga stanu podlewania.
 *
 * Zmienna informuje, czy w danym momencie trwa proces podlewania.
 * Pomaga zapobiec wielokrotnemu wywołaniu operacji podlewania (tzw. debouncing logiczny),
 * szczególnie w przypadkach, gdy polecenie może być szybko powtórzone (np. przez przycisk lub API).
 *
 * @note Używana jako zabezpieczenie przed powieleniem żądań podlewania.
 */
static bool isWatering = false;

/**
 * @brief Globalna konfiguracja użytkownika.
 * 
 * Zmienna przechowuje bieżącą konfigurację parametrów systemu podlewania, takich jak
 * czas podlewania, liczba próbek, opóźnienie odczytu i próg wilgotności.
 * 
 * @note Konfiguracja jest aktualizowana z poziomu HTTP API.
 */
volatile user_config_t user_config;

/**
 * @brief Kolejka zadań podlewania.
 * 
 * Używana do inicjowania procesu podlewania przez:
 * - task odczytu wilgotności (automatyczne podlewanie): `taskReadHumiditySensor(void*)`, 
 * - funkcje obsługi przerwania (ręczne podlewanie przycisiem): `on_manual_watering_press(void*)`.
 * - funkcje obsługującą żądanie z rest api (ręczne podlewanie z poziomu aplikacji): `water_post_handler(httpd_req_t *req)`
 */
volatile QueueHandle_t watering_queue = NULL;

/**
 * @brief Kolejka z ostatnim odczytem wilgotności.
 * 
 * Pozwala na asynchroniczne pobieranie danych o wilgotności przez serwer HTTP.
 */
volatile QueueHandle_t humidity_queue = NULL;

//-------------------------------- DEKLARACJE FUNKCJI --------------------------------------

/**
 * @brief Inicjalizacja konfiguracji użytkownika z pamięci NVS.
 *
 * Wczytuje konfigurację systemu podlewania z NVS. Jeśli brak danych w pamięci,
 * ustawia wartości domyślne i zapisuje je do NVS.
 */
void init_user_config();

/**
 * @brief Task odpowiedzialny za włączanie i wyłączanie pompy wody.
 *
 * Oczekuje na sygnał w kolejce `watering_queue`. Jeśli odbierze wartość `1`,
 * aktywuje przekaźnik pompy na czas określony w `user_config.watering_time`,
 * a następnie ją wyłącza. Ustawia flagę `isWatering` na czas działania pompy
 * w celu zapobieżenia wielokrotnym uruchomieniom.
 *
 * @param arg Parametr nieużywany.
 */
void taskWaterPump(void* arg);

/**
 * @brief Task cyklicznie odczytujący wilgotność gleby.
 *
 * Wykonuje `user_config.sample_count` odczytów z czujnika wilgotności,
 * oblicza ich średnią i umieszcza ją w `humidity_queue`.
 * Jeśli wilgotność przekroczy próg (`user_config.dry_threshold`),
 * dodaje sygnał podlewania do `watering_queue`.
 *
 * @param arg Parametr nieużywany.
 */
void taskReadHumiditySensor(void* arg);

/**
 * @brief Task do uruchomienia i obsługi serwera HTTP.
 *
 * Task pośredniczy w komunikacji między serwerem HTTP a resztą aplikacji.
 * Obecnie pełni funkcję placeholdera, ale może w przyszłości służyć do synchronizacji danych,
 * takich jak aktualna wilgotność.
 *
 * @param arg Parametr nieużywany.
 */
void taskHttpServer(void *arg);

/**
 * @brief Obsługa przerwania od przycisku ręcznego podlewania.
 *
 * Funkcja ISR (Interrupt Service Routine), która dodaje sygnał do `watering_queue`,
 * jeśli podlewanie nie jest już aktywne (`isWatering == false`).
 *
 * @note ISR — funkcja musi być szybka i nie może używać funkcji blokujących.
 *
 * @param arg Parametr nieużywany.
 */
static void IRAM_ATTR on_manual_watering_press(void* arg);

/**
 * @brief Odczyt i uśrednienie wilgotności z czujnika gleby.
 *
 * Wykonuje `user_config.sample_count` pomiarów z ADC,
 * każdy z opóźnieniem `user_config.read_delay` i wylicza średnią.
 * Następnie przelicza wartość surową na procentową.
 *
 * @param humidity Wskaźnik na zmienną, do której zostanie przypisana wilgotność [%].
 * @return `ESP_OK` w przypadku sukcesu, `ESP_FAIL` w przypadku błędu odczytu.
 */
esp_err_t read_average_humidity(float *humidity);

/**
 * @brief Konwersja surowej wartości ADC na wartość wilgotności [%].
 *
 * Zakres surowych wartości ustalony na podstawie kalibracji czujnika (950–2750).
 * Funkcja mapuje wynik pomiaru na przedział procentowy 0–100%.
 *
 * @param raw_value Surowa wartość odczytana z ADC.
 * @return Wilgotność gleby w procentach [0–100%].
 */
float raw_adc_to_humidity(int raw_value);

/**
 * @brief Główna funkcja aplikacji, punkt startowy programu na ESP32.
 * 
 * Funkcja `app_main()` wykonuje pełną inicjalizację systemu automatycznego podlewania roślin.
 * Odpowiada za konfigurację sprzętową, inicjalizację FreeRTOS, WiFi w trybie Access Point, 
 * odczyt i zapis konfiguracji użytkownika, oraz uruchomienie głównych zadań systemu.
 * 
 * Szczegółowe czynności wykonywane przez funkcję:
 * - Konfiguracja przetwornika analogowo-cyfrowego (ADC) do pomiaru wilgotności.
 * - Konfiguracja pinów cyfrowych: sterowanie pompą oraz przycisk ręcznego podlewania.
 * - Inicjalizacja kolejek FreeRTOS do komunikacji między taskami.
 * - Inicjalizacja systemu pamięci nieulotnej (NVS) do przechowywania konfiguracji.
 * - Załadowanie konfiguracji użytkownika z NVS lub ustawienie domyślnych wartości, jeśli brak danych.
 * - Instalacja obsługi przerwań dla przycisku do ręcznego podlewania.
 * - Inicjalizacja WiFi w trybie Access Point (AP).
 * - Utworzenie i uruchomienie trzech tasków FreeRTOS:
 *    - `taskWaterPump` — zarządzanie pracą pompy.
 *    - `taskReadHumiditySensor` — odczyt wilgotności gleby i sterowanie podlewaniem.
 *    - `taskHttpServer` — obsługa serwera HTTP (API).
 * - Uruchomienie serwera HTTP do obsługi zdalnych żądań.
 * 
 * @note Funkcja działa jako główna pętla programu, ale sama nie zawiera pętli — zadania realizują operacje cykliczne.
 */
void app_main(void) 
{	
	// Konfiguracja przetwornika analogowo-cyfrowego
    configAdc(); 
    
    // Konfiguracja wyświetlacza oled
    //configLcd(); 
    
    // Konfiguracja pinów cyfrowych
    configDigitalPin(GPIO_PUMP_CONTROL, GPIO_INTR_DISABLE, GPIO_MODE_OUTPUT);
    configDigitalPin(GPIO_MANUAL_WATERING_BUTTON, GPIO_MODE_INPUT, GPIO_INTR_POSEDGE); // inicjalizacja tutaj bo bez sensu w przerwaniu
    
    // Konfiguracja pinu analogowego
    configAnalogPin(GPIO_HUM_SENSOR);
    
    //Inicjalizacja kolejek
    watering_queue = xQueueCreate(1, sizeof(int));
    humidity_queue = xQueueCreate(1, sizeof(int));
    
    // Inicjalizacja NVS (Non-Volatile Storage)
    ESP_LOGI(TAG, "Inicjalizacja NVS...");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    
    // Wczytanie konfiguracji z NVS
    // Jeśli brak danych w konfiguracji - inicjalizujemy domyślną konfigurację
    if (config_load(&user_config) != ESP_OK) {
        ESP_LOGI(TAG, "Brak zapisanej konfiguracji. Używam wartosci domyslnych.");
        user_config.watering_time 	= DEFAULT_CONFIG_WATERING_TIME;
        user_config.sample_count 	= DEFAULT_CONFIG_SAMPLE_COUNT;
        user_config.read_delay 		= DEFAULT_CONFIG_READ_DELAY;
        user_config.dry_threshold 	= DEFAULT_CONFIG_DRY_THRESHOLD;
        
        // Zapis domyślnej konfiguracji
        ESP_ERROR_CHECK(config_save(&user_config));
    }

    // Wyświetlenie konfiguracji
    ESP_LOGI(TAG, "Konfiguracja: czas podlewania = %d ms, ilosc probek do sredniej = %d, czas pomiedzy pomiarami = %d s, prog wilgotnosci = %d.",
             user_config.watering_time, user_config.sample_count, user_config.read_delay, user_config.dry_threshold);
	
	// Konfiguracja przerwań
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT); // ustawienie flagi pod obsluge przerwan
    gpio_isr_handler_add(GPIO_MANUAL_WATERING_BUTTON, on_manual_watering_press, NULL);

	// Inicjalizacja wifi
    ESP_LOGI(TAG, "Inicjalizacja wifi");
    wifi_init_softap();

	// Inicjalizacja tasków
    xTaskCreate(&taskWaterPump, "taskWaterPump", 2048, NULL, 5, NULL);
    xTaskCreate(&taskReadHumiditySensor, "taskReadHumiditySensor", 2048, NULL, 5, NULL);
	xTaskCreate(&taskHttpServer, "taskHttpServer", 2048, NULL, 5, NULL);
	
	// Inicjalizacja serwera http
    start_webserver();
}

void init_user_config() 
{
    // Wczytanie konfiguracji z NVS
    // Jeśli brak danych w konfiguracji - inicjalizujemy domyślną konfigurację
    if (config_load(&user_config) != ESP_OK) {
        ESP_LOGI(TAG, "Brak zapisanej konfiguracji. Używam wartosci domyslnych.");
        user_config.watering_time = 1000;
        user_config.sample_count = 20;
        user_config.read_delay = 100;
        user_config.dry_threshold = 50;
        
        // Zapis domyślnej konfiguracji
        ESP_ERROR_CHECK(config_save(&user_config));
    }

    // Wyświetlenie konfiguracji
    ESP_LOGI(TAG, "Konfiguracja: czas podlewania = %d ms, ilosc probek do sredniej = %d, czas pomiedzy pomiarami = %d s, prog wilgotnosci = %d.",
             user_config.watering_time, user_config.sample_count, user_config.read_delay, user_config.dry_threshold);
}

void taskHttpServer(void *arg) {
    ESP_LOGI(TAG, "Start HTTP server task");

    while(1) 
    {

		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void taskWaterPump(void* arg)
{
    uint32_t watering_signal;	// Sygnał podlewania z kolejki, 1 - podlej, 0 - nie podlewaj
 
    while(1)
    {       
		// Jeżeli przyjmie dane z kolejki i wartość podana będzie równa 1, to podlej przez ustalony czas
        if (xQueueReceive(watering_queue, &watering_signal, portMAX_DELAY) 
        		&& watering_signal == 1)
        {
			// Ustaw flage podlewania
            isWatering = true;
            
            // Rozpocznij podlewanie
            gpio_set_level(GPIO_PUMP_CONTROL, 1);
            ESP_LOGI(TAG, "Rozpoczeto podlewanie.");

			// Odczekaj ustalony czas
            vTaskDelay(pdMS_TO_TICKS(user_config.watering_time));

			// Zatrzymaj podlewanie
            gpio_set_level(GPIO_PUMP_CONTROL, 0);
            ESP_LOGI(TAG, "Zakonczono podlewanie.");
            
            // Zwolnij flage podlewania
            isWatering = false;
        }
    } 
}

void taskReadHumiditySensor(void* arg) 
{
    uint32_t watering_signal = 1;  // Sygnał podlewania, 1 - podlej
 
    while (1) 
    {
        float humidity = 0.0f;

		// Odczytaj średnią wilgotność
        esp_err_t err = read_average_humidity(&humidity);
        
        // Jeśli wystąpił błąd, pomiń iteracje pętli
        if(err != ESP_OK) continue;
        
        // Wyślij wilgotność do kolejki, ewentualnie nadpisz starą wartość wilgotności
        xQueueOverwrite(humidity_queue, &humidity);
        
		// Wyświetl wilgotność
        ESP_LOGI(TAG, "Humidity = %f", humidity);
        //display_humidity(humidity); // wyświetlacz
        
        // Określ czy należy podlać
        bool water_needed = (HUM_THRESHOLD_REVERSE) ? (humidity >= user_config.dry_threshold) : (humidity <= user_config.dry_threshold);

		// Jeśli podlewanie wymagane
        if (water_needed) {
            ESP_LOGI(TAG, "[taskReadHumiditySensor] Dodaje sygnal podlewania do kolejki.");
            xQueueSend(watering_queue, &watering_signal, portMAX_DELAY);
            //xSemaphoreTake(watering_semaphore, portMAX_DELAY);
        }
    }
}
 
static void IRAM_ATTR on_manual_watering_press(void* arg) 
{
    uint32_t watering_signal = 1;  // Sygnał podlewania, 1 - podlej

	// Jeśli podlewanie jest zatrzymane
    if(!isWatering)
    	// Dodaj sygnał podlewania do kolejki
        xQueueSendFromISR(watering_queue, &watering_signal, NULL);
        
    // xSemaphoreTakeFromISR(watering_semaphore, NULL);
}

esp_err_t read_average_humidity(float *humidity) 
{
    int sum = 0;
 
    for (int i = 0; i < user_config.sample_count; i++) 
    {    
        // Odczytaj wartość z przetwornika
        int adc_reading = 0;
        esp_err_t err = readAnalogValue(GPIO_HUM_SENSOR, &adc_reading);

		// Sprawdź błędy
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Blad podczas odczytu z ADC.");
            return err;
        }

        sum += adc_reading;
        
        // Odczekaj określony czas przed następnym pomiarem
        vTaskDelay(pdMS_TO_TICKS(user_config.read_delay));
    }
    
    // Zamień surową wartość z ADC na procentowy zapis
    *humidity = raw_adc_to_humidity(sum / user_config.sample_count);
    
    return ESP_OK;
}

float raw_adc_to_humidity(int raw_value) 
{	
    // Zakres pomiarowy przetwornika
    int min_adc = 950;
    int max_adc = 2750;

    // Pominięcie pomiarów które wyszły poza ustalony zakres
    if(raw_value < min_adc) raw_value = min_adc; 
    if(raw_value > max_adc) raw_value = max_adc; 

    // Przetworzenie skali min_adc-max_adc na wartość w procentach
    float humidity = 100 - ((float)(raw_value - min_adc) / (max_adc - min_adc)) * 100.0; 

    return humidity;
}




