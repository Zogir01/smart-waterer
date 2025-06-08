/**
 * @file io_util.h
 * @brief Biblioteka projektu do konfiguracji oraz odczytu wartości z pinów analogowych/cyfrowych.
 * 
 * @todo Zrobić obsługę zarówno ADC1 oraz ADC2. Napisać warunki preprocesora sprawdzające platformę esp,
 * gdyż niektóre ESP mają tylko jeden przetwornik. W ten sposób kod będzie "wieloplatformowy".
 * 
 * @todo W funkcji gpio_to_adc_channel, lub gdzie indziej, można by sprawdzać czy podany gpio posiada w ogóle kanał adc,
 * może być tak że posiada kanał ale jest to kanał dla ADC2, którego na ten moment nie wykorzystujemy.
 * Wiadomo zamiast robić zabezpieczenia, można by po prostu uważać i patrzyć na dokumentację, ale jeśli to biblioteka,
 * to taki mechanizm byłby chyba spoko.
 * 
 * @todo Pomyśleć nad wygenerowaniem dokumentacji Doxygen.
 * 
 * @warning Użycie tej biblioteki wymaga wstępnej konfiguracji ADC za pomocą funkcji configAdc().
 * 
 * @date 2025-03-28
 */

#ifndef IO_UTIL_H
#define IO_UTIL_H

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"

/**
 * @brief Funkcja konfiguruje pin cyfrowy.
 * 
 * Ta funkcja konfiguruje dany pin cyfrowy w wybranym trybie oraz typie przerwania.
 * 
 * @param outputPin Numer pinu GPIO do konfiguracji.
 * @param type Typ przerwania dla pinu (np. Rising, Falling).
 * @param mode Tryb pinu (wejście/wyjście).
 * @return ESP_OK w przypadku sukcesu, inny kod błędu w przypadku niepowodzenia.
 */
esp_err_t configDigitalPin(gpio_num_t outputPin, gpio_int_type_t type, gpio_mode_t mode);

/**
 * @brief Funkcja konfigurująca jednostkę ADC.
 * 
 * Funkcja ta inicjalizuje jednostkę ADC i ustawia jej parametry.
 * 
 * @return ESP_OK w przypadku powodzenia, inny kod błędu w przypadku niepowodzenia.
 */
esp_err_t configAdc();

/**
 * @brief Funkcja konfigurująca kanał ADC.
 * 
 * Funkcja ta ustawia odpowiedni kanał ADC w zależności od numeru GPIO (numeru kanału).
 * 
 * @return ESP_OK w przypadku powodzenia, inny kod błędu w przypadku niepowodzenia.
 */
esp_err_t configAnalogPin(gpio_num_t gpio_num);

/**
 * @brief Funkcja odczytująca wartość z pinu analogowego.
 * 
 * Funkcja ta wykorzystuje gpio_to_adc_channel() w celu uzyskania numeru kanału adc przypisanego
 * do podanego numeru GPIO.
 * 
 * @param gpio_num Port GPIO do oczytania.
 * @return ESP_OK w przypadku powodzenia, inny kod błędu w przypadku niepowodzenia.
 */
esp_err_t readAnalogValue(gpio_num_t gpio_num, int* value);

#endif // IO_UTIL_H
