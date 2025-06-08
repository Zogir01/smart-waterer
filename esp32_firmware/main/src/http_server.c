#include "../inc/http_server.h"
#include "../inc/user_config.h"
#include "esp_http_server.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "cJSON.h"
#include <string.h>
#include <fcntl.h>
#include <math.h>

static const char *TAG = "http_server";

extern volatile user_config_t user_config;
//extern volatile float current_humidity;
//extern volatile bool isWatering;
extern volatile QueueHandle_t watering_queue;
extern volatile QueueHandle_t humidity_queue;

// Handler obsługujący zwrócenie prostej strony html
// GET /
esp_err_t root_get_handler(httpd_req_t *req)
{
	// Prosta stronka html do testów
	// Prosta stronka html wysyłająca zapytanie GET /api/humidity co sekunde oraz wyświetlająca wilgotność
	const char *html =
    "<!DOCTYPE html>"
    "<html><head><title>ESP32 Wilgotnosc</title></head>"
    "<body>"
    "<h1>Aktualna wilgotnosc:</h1>"
    "<p id=\"humidity\">Ładowanie...</p>"

    "<button id=\"waterBtn\">Podlej teraz</button>"
    "<p id=\"waterResult\"></p>"

    "<script>"
    "function fetchHumidity() {"
    "  fetch('/api/humidity')"
    "    .then(response => response.json())"
    "    .then(data => {"
    "      document.getElementById('humidity').innerText = data.humidity_percent + ' %';"
    "    })"
    "    .catch(err => {"
    "      document.getElementById('humidity').innerText = 'Blad odczytu';"
    "    });"
    "}"
    "document.getElementById('waterBtn').addEventListener('click', function() {"
    "  fetch('/api/water', { method: 'POST' })"
    "    .then(response => {"
    "      if (response.ok) return response.json();"
    "      else return response.json().then(err => Promise.reject(err));"
    "    })"
    "    .then(data => {"
    "      document.getElementById('waterResult').innerText = data.message || 'Podlewanie rozpoczęte';"
    "    })"
    "    .catch(err => {"
    "      document.getElementById('waterResult').innerText = err.error || 'Blad podlewania';"
    "    });"
    "});"
    "setInterval(fetchHumidity, 1000);"
    "fetchHumidity();"
    "</script>"
    "</body></html>";

	// Ustawienie nagłówka odpowiedzi (Content-Type) na html
    httpd_resp_set_type(req, "text/html");
    
    // Wysłanie odpowiedzi
    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
    
    return ESP_OK;
}

// Handler obsługujący zdalne podlewanie
// POST api/water
esp_err_t water_post_handler(httpd_req_t *req) 
{
	uint32_t watering_signal = 1;  // Sygnał podlewania, 1 - podlej

	// Jeśli podlewanie trwa: zwróć error 409 - conflict
/*    if(isWatering) {
        httpd_resp_set_status(req, "409 Conflict");
        httpd_resp_set_type(req, "application/json");
        const char *resp = "{\"error\":\"Podlewanie jest już w toku.\"}";
        httpd_resp_sendstr(req, resp);
        return ESP_OK;
	}*/
	
    // Dodaj sygnał podlewania do kolejki
    xQueueSend(watering_queue, &watering_signal, portMAX_DELAY);
    
    // Wyślij odpowiedź
	httpd_resp_set_type(req, "application/json");
	httpd_resp_set_status(req, "200 OK");
	const char *resp = "{\"message\":\"Podlewanie rozpoczete.\"}";
    httpd_resp_sendstr(req, resp);
    
	return ESP_OK;
}

// Handler obsługujący wysłanie aktualnej wilgotności do clienta
// GET api/humidity
esp_err_t humidity_get_handler(httpd_req_t *req)
{
	// Inicjalizacja obiektu JSON
	cJSON *json = cJSON_CreateObject();
    if (!json) return ESP_FAIL;
    
    float humidity = 0.0f;
    
    // Odczyt aktualnej wilgotnosci z kolejki
	// Jeśli funkcja xQueueReceive zwróci pdFALSE, oznacza to, że nie ma aktualnej wilgotności
	// W takim przypadku zwróć błąd w response.
	if (xQueueReceive(humidity_queue, &humidity, portMAX_DELAY) 
	== pdFALSE) {
		httpd_resp_set_status(req, "409 Conflict");
        httpd_resp_set_type(req, "application/json");
        const char *resp = "{\"error\":\"Brak aktualnej wilgotnosci.\"}";
        httpd_resp_sendstr(req, resp);
        return ESP_OK;
	} 
	
    // Zapis wilgotności do JSON
    cJSON_AddNumberToObject(json, "humidity_percent", humidity);
    
    // Konwersja obiektu JSON na nieformatowany tekst
	const char *json_str = cJSON_PrintUnformatted(json);
    if (!json_str) {
        cJSON_Delete(json);
        return ESP_FAIL;
    }
    
    // Ustawienie nagłówka odpowiedzi (Content-Type) na json
    httpd_resp_set_type(req, "application/json");
    
    // Wysłanie odpowiedzi
    httpd_resp_sendstr(req, json_str);
    
    // Zwolnienie zasobów
    cJSON_free((void *)json_str);
    cJSON_Delete(json);
    
    return ESP_OK;
}

// Handler obsługujący zmiane konfiguracji użytkownika
// POST api/config
esp_err_t config_post_handler(httpd_req_t *req) 
{
	// Bufor danych 
    char buf[100];
    
    // Ustawienie bufora
    int ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret <= 0) return ESP_FAIL;

	// Konwersja tekstu na obiekt JSON
    cJSON *json = cJSON_Parse(buf);
    if (!json) return ESP_FAIL;

	// Zmiana istniejącej konfiguracji w programie
    user_config.watering_time = cJSON_GetObjectItem(json, "watering_time")->valueint;
    user_config.sample_count = cJSON_GetObjectItem(json, "sample_count")->valueint;
    user_config.read_delay = cJSON_GetObjectItem(json, "read_delay")->valueint;
    user_config.dry_threshold = cJSON_GetObjectItem(json, "dry_threshold")->valueint;

	// Zapis nowej konfiguracji do pamięci nvs
    config_save(&user_config);
    
    // Zwolnienienie zasobów
    cJSON_Delete(json);

	// Wysłanie odpowiedzi
    httpd_resp_sendstr(req, "OK");
    
    return ESP_OK;
}

// Handler obsługujący wysłanie konfiguracji użytkownika do clienta
// GET api/config
esp_err_t config_get_handler(httpd_req_t *req) 
{
	// Inicjalizacja obiektu JSON
	cJSON *json = cJSON_CreateObject();
    if (!json) return ESP_FAIL;

    // Zapisanie zmiennych z konfiguracji użytkownika do obiektu JSON
    cJSON_AddNumberToObject(json, "watering_time", user_config.watering_time);
    cJSON_AddNumberToObject(json, "sample_count", user_config.sample_count);
    cJSON_AddNumberToObject(json, "read_delay", user_config.read_delay);
    cJSON_AddNumberToObject(json, "dry_threshold", user_config.dry_threshold);

	// Konwersja obiektu JSON na nieformatowany tekst
	const char *json_str = cJSON_PrintUnformatted(json);
    if (!json_str) {
        cJSON_Delete(json);
        return ESP_FAIL;
    }
    
    // Ustawienie nagłówka odpowiedzi (Content-Type) na json
    httpd_resp_set_type(req, "application/json");
    
    // Wysłanie odpowiedzi
    httpd_resp_sendstr(req, json_str);

    // Zwolnienie zasobów
    cJSON_free((void *)json_str);
    cJSON_Delete(json);

    return ESP_OK;
}

httpd_uri_t uri_root_get = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = root_get_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_water_post = {
    .uri      = "/api/water",
    .method   = HTTP_POST,
    .handler  = water_post_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_humidity_get = {
    .uri      = "/api/humidity",
    .method   = HTTP_GET,
    .handler  = humidity_get_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_config_get = {
    .uri      = "/api/config",
    .method   = HTTP_GET,
    .handler  = config_get_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_config_post = {
    .uri      = "/api/config",
    .method   = HTTP_POST,
    .handler  = config_post_handler,
    .user_ctx = NULL
};

httpd_handle_t start_webserver(void)
{
    // Domyślna konfiguracja
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	// Uchwyt - handler do serwera
    httpd_handle_t server = NULL;

    // Startowanie serwera http
    if (httpd_start(&server, &config) == ESP_OK) {
        // Zarejestrowanie handlerów
        httpd_register_uri_handler(server, &uri_root_get);
        httpd_register_uri_handler(server, &uri_water_post);
        httpd_register_uri_handler(server, &uri_humidity_get);
        httpd_register_uri_handler(server, &uri_config_get);
        httpd_register_uri_handler(server, &uri_config_post);
    }

    return server;
}

void stop_webserver(httpd_handle_t server)
{
    if (server) {
        httpd_stop(server);
    }
}