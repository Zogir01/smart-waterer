/**
 * @file http_server.h
 * @brief Plik nagłówkowy zawierający deklaracje funkcji i struktur do obsługi serwera HTTP na ESP32.
 * 
 * Plik definiuje endpointy REST API do obsługi systemu podlewania: aktualna wilgotność, konfiguracja parametrów
 * oraz ręczne wywołanie podlewania. Zawiera deklaracje handlerów HTTP oraz funkcji `start_webserver()` i `stop_webserver()`.
 * 
 * API udostępniane przez ten moduł:
 * - `GET /` – statyczna strona HTML z interfejsem użytkownika,
 * - `GET /api/humidity` – pobranie aktualnej wilgotności,
 * - `POST /api/water` – ręczne uruchomienie podlewania,
 * - `GET /api/config` – pobranie aktualnej konfiguracji,
 * - `POST /api/config` – zapis nowej konfiguracji.
 * 
 * @note Korzystanie z tego modułu wymaga skonfigurowanej sieci (np. Access Pointa) oraz obsługi JSON przez bibliotekę cJSON.
 * 
 * @warning Funkcje wymagają dostępu do zewnętrznych zasobów, takich jak kolejki `watering_queue` oraz `humidity_queue`.
 */
#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "esp_http_server.h"

/**
 * @brief Obsługuje żądanie GET na stronie głównej.
 * 
 * Wyświetla stronę HTML klienta webowego z aktualną wilgotnością gleby, przyciskiem do zdalnego podlewania,
 * formularzem do zmiany konfiguracji oraz wyświetleniem aktualnych ustawień.
 * 
 * @param req Wskaźnik do struktury żądania HTTP.
 * @return ESP_OK jeśli odpowiedź została pomyślnie wysłana.
 */
esp_err_t root_get_handler(httpd_req_t *req);

/**
 * @brief Obsługuje żądanie POST do rozpoczęcia podlewania.
 * 
 * Dodaje sygnał podlewania do kolejki `watering_queue`. Jeśli kolejka jest pełna przez ponad 100 ms,
 * zwracany jest błąd 503.
 * 
 * @param req Wskaźnik do struktury żądania HTTP.
 * @return ESP_OK zawsze (nawet w przypadku błędu logicznego, odpowiedź HTTP zawiera odpowiedni status).
 */
esp_err_t water_post_handler(httpd_req_t *req);

/**
 * @brief Obsługuje żądanie GET aktualnej wilgotności.
 * 
 * Odczytuje najnowszą wartość wilgotności z kolejki `humidity_queue` i wysyła ją w formacie JSON.
 * Jeśli wilgotność nie jest dostępna w ciągu 100 ms, zwracany jest błąd 409.
 * 
 * @param req Wskaźnik do struktury żądania HTTP.
 * @return ESP_OK jeśli odpowiedź została pomyślnie wysłana.
 */
esp_err_t humidity_get_handler(httpd_req_t *req);

/**
 * @brief Obsługuje żądanie POST zmieniające konfigurację użytkownika.
 * 
 * Odczytuje dane JSON zawierające nowe ustawienia użytkownika (czas podlewania, liczba próbek,
 * opóźnienie odczytu, próg wilgotności) i zapisuje je do zmiennej `user_config` oraz do NVS.
 * Waliduje zakresy parametrów – w przypadku błędnych danych zwraca błąd 400.
 * 
 * @param req Wskaźnik do struktury żądania HTTP.
 * @return ESP_OK jeśli dane zostały przyjęte lub jeśli odpowiedź z błędem została wysłana.
 */
esp_err_t config_post_handler(httpd_req_t *req);

/**
 * @brief Obsługuje żądanie GET konfiguracji użytkownika.
 * 
 * Wysyła bieżącą konfigurację użytkownika zapisaną w `user_config` w formacie JSON.
 * 
 * @param req Wskaźnik do struktury żądania HTTP.
 * @return ESP_OK jeśli odpowiedź została pomyślnie wysłana.
 */
esp_err_t config_get_handler(httpd_req_t *req);

/**
 * @brief Uruchamia serwer HTTP i rejestruje endpointy.
 * 
 * Tworzy instancję serwera HTTP i rejestruje wszystkie zdefiniowane ścieżki (GET/POST).
 * 
 * @return Wskaźnik do uchwytu serwera HTTP, lub NULL jeśli uruchomienie się nie powiodło.
 */
httpd_handle_t start_webserver(void);

/**
 * @brief Zatrzymuje działanie serwera HTTP.
 * 
 * Zatrzymuje serwer HTTP, jeśli jest uruchomiony.
 * 
 * @param server Uchwyt do działającego serwera HTTP.
 */
void stop_webserver(httpd_handle_t server);

#endif // HTTP_SERVER.H
