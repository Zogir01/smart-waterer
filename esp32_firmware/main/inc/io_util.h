/**
 * @file io_util.h
 * @brief Plik nagłówkowy z funkcjami do obsługi pinów cyfrowych/analogowych.
 * 
 * @todo W funkcji gpio_to_adc_channel, lub gdzie indziej, można by sprawdzać czy podany gpio posiada w ogóle kanał adc,
 * może być tak że posiada kanał ale jest to kanał dla ADC2, którego na ten moment nie wykorzystujemy.
 * Wiadomo zamiast robić zabezpieczenia, można by po prostu uważać i patrzyć na dokumentację, ale jeśli to biblioteka,
 * to taki mechanizm byłby chyba spoko.
 * 
 * @note Użycie funkcji z tego pliku wymaga wstępnej konfiguracji ADC za pomocą funkcji configAdc().
 */

#ifndef IO_UTIL_H
#define IO_UTIL_H

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_err.h"

/**
 * @brief Konfiguruje pin GPIO jako cyfrowy.
 * 
 * Funkcja ustawia tryb pinu GPIO, typ przerwania (jeśli jest) oraz maskę bitową pinu.
 * Używana do ustawienia pinów jako wejście lub wyjście cyfrowe.
 * 
 * @param outputPin Numer GPIO pinu do konfiguracji.
 * @param type Typ przerwania GPIO (np. bez przerwania, narastające zbocze).
 * @param mode Tryb GPIO (np. wejście, wyjście).
 * @return esp_err_t Zawsze ESP_OK (póki co brak obsługi błędów).
 */
esp_err_t configDigitalPin(gpio_num_t outputPin, gpio_int_type_t type, gpio_mode_t mode);

/**
 * @brief Inicjalizuje i konfiguruje jednostkę ADC1 w trybie oneshot.
 * 
 * Funkcja ustawia źródło zegara i wyłącza tryb ULP, następnie tworzy uchwyt ADC.
 * Umożliwia późniejszy odczyt analogowy z wybranych pinów.
 * 
 * @return esp_err_t Kod błędu ESP_OK przy powodzeniu.
 */
esp_err_t configAdc();

/**
 * @brief Konfiguruje pin GPIO jako wejście analogowe ADC.
 * 
 * Funkcja mapuje numer GPIO na kanał i jednostkę ADC, ustawia parametry kanału ADC
 * takie jak rozdzielczość (bitwidth) oraz tłumienie (attenuation).
 * 
 * @param gpio_num Numer GPIO, który ma zostać skonfigurowany jako analogowy.
 * @return esp_err_t ESP_OK przy powodzeniu, ESP_ERR_INVALID_STATE jeśli ADC nie został jeszcze zainicjalizowany.
 */
esp_err_t configAnalogPin(gpio_num_t gpio_num);

/**
 * @brief Odczytuje wartość analogową z wybranego pinu GPIO.
 * 
 * Funkcja mapuje GPIO na kanał ADC, a następnie wykonuje odczyt wartości analogowej w trybie oneshot.
 * Zapisuje wynik do wskazanej zmiennej.
 * 
 * @param gpio_num Numer GPIO pinu analogowego do odczytu.
 * @param value Wskaźnik na zmienną typu int, gdzie zostanie zapisana odczytana wartość ADC.
 * @return esp_err_t Kod błędu operacji; ESP_OK przy powodzeniu.
 */
esp_err_t readAnalogValue(gpio_num_t gpio_num, int* value);

#endif // IO_UTIL_H
