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

//#include "http_server.h"
#include "../inc/wifi_ap.h"

// ------------------------------- ZMIENNE GLOBALNE ----------------------------------------
static const char *TAG = "MAIN"; // TAG błędów dla main.c

static bool isWatering = false; // globalna flaga (przeciw debouncing'owi)

volatile QueueHandle_t humidity_queue = NULL; // handlery kolejek
volatile QueueHandle_t watering_queue = NULL; //
volatile user_config_t user_config;

//-------------------------------- DEKLARACJE FUNKCJI --------------------------------------

/**
 * @brief [Tutaj wstawić skrócony opis funkcji]
 * 
 * [Tutaj wstawić opis funkcji]
 * 
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @return [Tutaj wstawić opis co funkcja zwraca.]
 */
void taskWaterPump(void* arg);

/**
 * @brief [Tutaj wstawić skrócony opis funkcji]
 * 
 * [Tutaj wstawić opis funkcji]
 * 
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @return [Tutaj wstawić opis co funkcja zwraca.]
 */
void taskReadHumiditySensor(void* arg);

/**
 * @brief [Tutaj wstawić skrócony opis funkcji]
 * 
 * [Tutaj wstawić opis funkcji]
 * 
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @return [Tutaj wstawić opis co funkcja zwraca.]
 */
void taskHttpServer(void *arg);

/**
 * @brief [Tutaj wstawić skrócony opis funkcji]
 * 
 * [Tutaj wstawić opis funkcji]
 * 
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @return [Tutaj wstawić opis co funkcja zwraca.]
 */
static void IRAM_ATTR on_manual_watering_press(void* arg);

/**
 * @brief [Tutaj wstawić skrócony opis funkcji]
 * 
 * [Tutaj wstawić opis funkcji]
 * 
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @return [Tutaj wstawić opis co funkcja zwraca.]
 */
esp_err_t read_average_humidity(float *humidity);

/**
 * @brief [Tutaj wstawić skrócony opis funkcji]
 * 
 * [Tutaj wstawić opis funkcji]
 * 
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @return [Tutaj wstawić opis co funkcja zwraca.]
 */
float raw_adc_to_humidity(int raw_value);

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
    watering_queue = xQueueCreate(10, sizeof(int));
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

// Task będący pośrednikiem pomiędzy serwerem http, a resztą programu
void taskHttpServer(void *arg) {
    ESP_LOGI(TAG, "Start HTTP server task");

    while(1) 
    {
/*		float humidity = 0.0f;
		
		// Odczyt aktualnej wilgotności
		if (xQueueReceive(humidity_queue, &humidity, portMAX_DELAY)) {
			// Zapis aktualnej wilgotności do zmiennej, wykorzystywanej przez http server
			current_humidity = humidity;
		} */
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

// Task odpowiedzialny za nasluchiwanie i wlaczanie pompy wody
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

// Task odczytujący pomiary wilgotności 
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
 
// Obsługa przerwań na zmiane stanu przycisku do ręcznego podlewania
static void IRAM_ATTR on_manual_watering_press(void* arg) 
{
    uint32_t watering_signal = 1;  // Sygnał podlewania, 1 - podlej

	// Jeśli podlewanie jest zatrzymane
    if(!isWatering)
    	// Dodaj sygnał podlewania do kolejki
        xQueueSendFromISR(watering_queue, &watering_signal, NULL);
        
    // xSemaphoreTakeFromISR(watering_semaphore, NULL);
}

// Odczyt średniej wilgotności z ADC
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

// Zamienia surową wartość wilgotności z ADC na procentowy zapis
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




