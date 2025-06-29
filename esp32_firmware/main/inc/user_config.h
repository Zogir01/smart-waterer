/**
 * @file user_config.h
 * @brief Funkcje do zapisu i odczytu konfiguracji użytkownika w pamięci NVS.
 */
#ifndef USER_CONFIG_H
#define USER_CONFIG_H

#include "esp_err.h"

/**
 * @brief Struktura przechowująca konfigurację systemu podlewania.
 *
 * Użytkownik może modyfikować te ustawienia z poziomu aplikacji mobilnej
 * lub interfejsu REST. Dane są przechowywane w pamięci NVS.
 */
typedef struct {
	/**
	 * @brief Czas trwania podlewania w milisekundach.
	 *
	 * Po spełnieniu warunku wilgotności, system uruchamia zawór na ten czas.
	 */
    int watering_time; 
    
    /**
	 * @brief Minimalny czas (w milisekundach) pomiędzy kolejnymi cyklami podlewania.
	 *
	 * Po wykonaniu podlewania system odczekuje co najmniej ten czas,
	 * zanim ponownie rozważy uruchomienie zaworu.
	 */
	int watering_interval;
    
    /**
     * @brief Ilość pomiarów wilgotności do uśrednienia.
     *
     * Przed podjęciem decyzji o podlewaniu wykonywanych jest kilka pomiarów,
     * a następnie liczona jest średnia.
     */
    int sample_count;   // Ilość pomiarów wilgotności do uśrednienia
    
    /**
     * @brief Czas w milisekundach między kolejnymi pomiarami wilgotności.
     *
     * Wpływa na szybkość działania algorytmu decyzyjnego.
     */
    int read_delay;		// Czas pomiędzy pomiarami wilgotności [ms]
    
    /**
     * @brief Próg wilgotności (%), poniżej którego uruchamiane jest podlewanie.
     *
     * Jeżeli średnia wilgotność spadnie poniżej tego progu, następuje podlewanie.
     */
    int dry_threshold;	// Próg wilgotności dla którego podlewać [%]
    
} user_config_t;

/**
 * @brief Zapisuje konfigurację użytkownika do pamięci NVS.
 * 
 * Funkcja otwiera przestrzeń nazw NVS, zapisuje poszczególne pola struktury `user_config_t`
 * jako wartości całkowite (int32) i zatwierdza zmiany w pamięci nieulotnej.
 * 
 * @param config Wskaźnik na strukturę `user_config_t` zawierającą parametry konfiguracji do zapisania.
 * @return esp_err_t Kod błędu z operacji NVS. ESP_OK oznacza powodzenie.
 */
esp_err_t config_save(const user_config_t *config);

/**
 * @brief Wczytuje konfigurację użytkownika z pamięci NVS.
 * 
 * Funkcja otwiera przestrzeń nazw NVS w trybie tylko do odczytu i pobiera wartości
 * poszczególnych parametrów konfiguracji zapisane wcześniej funkcją `config_save`.
 * 
 * @param config Wskaźnik na strukturę `user_config_t`, do której zostaną zapisane odczytane wartości.
 * @return esp_err_t Kod błędu z operacji NVS. ESP_OK oznacza powodzenie.
 */
esp_err_t config_load(user_config_t *config);

#endif // USER_CONFIG_H