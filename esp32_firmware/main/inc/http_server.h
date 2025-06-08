#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "esp_http_server.h"

/**
 * @brief [Tutaj wstawić skrócony opis funkcji]
 * 
 * [Tutaj wstawić opis funkcji]
 * 
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @return [Tutaj wstawić opis co funkcja zwraca.]
 */
esp_err_t get_handler(httpd_req_t *req);

/**
 * @brief [Tutaj wstawić skrócony opis funkcji]
 * 
 * [Tutaj wstawić opis funkcji]
 * 
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @return [Tutaj wstawić opis co funkcja zwraca.]
 */
esp_err_t post_handler(httpd_req_t *req);

/**
 * @brief [Tutaj wstawić skrócony opis funkcji]
 * 
 * [Tutaj wstawić opis funkcji]
 * 
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @return [Tutaj wstawić opis co funkcja zwraca.]
 */
httpd_handle_t start_webserver(void);

/**
 * @brief [Tutaj wstawić skrócony opis funkcji]
 * 
 * [Tutaj wstawić opis funkcji]
 * 
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @param [Tutaj wstawić opis parametrów (jeśli są)]
 * @return [Tutaj wstawić opis co funkcja zwraca.]
 */
void stop_webserver(httpd_handle_t server);


#endif // HTTP_SERVER.H
