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
 * @brief [Tutaj wstawić skrócony opis funkcji]
 * 
 * [Tutaj wstawić opis funkcji]
 * 
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @return [Tutaj wstawić opis co funkcja zwraca.]
 */
esp_err_t config_save(const user_config_t *config);

/**
 * @brief [Tutaj wstawić skrócony opis funkcji]
 * 
 * [Tutaj wstawić opis funkcji]
 * 
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @return [Tutaj wstawić opis co funkcja zwraca.]
 */
esp_err_t config_load(user_config_t *config);

#endif // USER_CONFIG_H