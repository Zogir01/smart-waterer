#include "../inc/config.h"
#include "../inc/io_util.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"
#include "esp_log.h"

/**
 * @brief TAG do logów dla tego modułu.
 */
static const char *TAG = "IO_UTIL"; // TAG błędów dla io_util.c

/**
 * @brief Uchwyt do jednostki ADC skonfigurowanej w trybie oneshot.
 * 
 * Przechowuje kontekst ADC potrzebny do konfiguracji kanałów i odczytów.
 */
static adc_oneshot_unit_handle_t adc_handle = NULL;

esp_err_t configDigitalPin(gpio_num_t outputPin, gpio_int_type_t type, gpio_mode_t mode) 
{
    ESP_LOGI(TAG, "Konfiguracja pinu cyfrowego GPIO %d.", outputPin);

    gpio_config_t io_conf = {};
    io_conf.intr_type = type;
    io_conf.mode = mode;
    io_conf.pin_bit_mask = (1ULL << outputPin);
    // io_conf.pull_down_en = 0;
    // io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    
    // póki co nie zwracamy błędów
    return ESP_OK;
}
 
esp_err_t configAdc() 
{
    ESP_LOGD(TAG, "Konfiguracja przetwornika ADC.");

    // Konfiguracja jednostki ADC1
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = false
    };
 
    // Inicjalizacja jednostki ADC1
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    return ESP_OK;
}

esp_err_t configAnalogPin(gpio_num_t gpio_num) 
{
    adc_channel_t adc_channel; 
    adc_unit_t adc_unit;
    
    ESP_LOGD(TAG, "Konfiguracja pinu analogowego GPIO %d w jednostce ADC %d, na kanale %d, ", 
        gpio_num, adc_unit, adc_channel);
    
    if (adc_handle == NULL) {
        ESP_LOGE(TAG, "Nie mozna skonfigurowac pinu analogowego, gdyż 'adc_handle' jest null.");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_ERROR_CHECK(adc_oneshot_io_to_channel(gpio_num, &adc_unit, &adc_channel));
    
    // Konfiguracja kanału ADC
    adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11 // Dla napięcia do ~3.9V
    };
 
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, adc_channel, &channel_config));

    return ESP_OK;
}

esp_err_t readAnalogValue(gpio_num_t gpio_num, int* value) 
{
    adc_channel_t adc_channel; 
    adc_unit_t adc_unit;

    if(value == NULL) {
		ESP_LOGE(TAG, "Nie mozna odczytac z adc, wskaznik 'value' jest null.");
		return ESP_ERR_INVALID_ARG;
	}

    if (adc_handle == NULL) {
        ESP_LOGE(TAG, "Nie mozna odczytac z adc, gdyż 'adc_handle' jest null.");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_ERROR_CHECK(adc_oneshot_io_to_channel(gpio_num, &adc_unit, &adc_channel));

    esp_err_t err = adc_oneshot_read(adc_handle, adc_channel, value);
    if (err != ESP_OK) return err;
    
    ESP_LOGD(TAG, "Odczyt z pinu analogowego GPIO %d w jednostce ADC %d, na kanale %d = %d ", 
        gpio_num, adc_unit, adc_channel, *value);

    return ESP_OK;
}