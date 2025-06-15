#include "../inc/http_server.h"
#include "../inc/user_config.h"
#include "../inc/config.h"
#include "esp_http_server.h"
#include "freertos/projdefs.h"
#include "cJSON.h"
#include <string.h>
#include <fcntl.h>
#include <math.h>

/**
 * @brief TAG do logów dla tego modułu.
 */
static const char *TAG = "http_server";

extern volatile user_config_t user_config;
extern volatile QueueHandle_t watering_queue;
extern volatile QueueHandle_t humidity_queue;

esp_err_t root_get_handler(httpd_req_t *req)
{
	// Testowa strona html wysyłająca zapytanie GET /api/humidity co sekunde oraz wyświetlająca wilgotność

const char *html =
"<!DOCTYPE html>"
"<html><head><title>ESP32 Wilgotnosc</title></head>"
"<body>"
"<h1>Aktualna wilgotnosc:</h1>"
"<p id=\"humidity\">Ładowanie...</p>"

"<button id=\"waterBtn\">Podlej teraz</button>"
"<p id=\"waterResult\"></p>"

"<h2>Zmien konfiguracje</h2>"
"<form id=\"configForm\">"
"  <label>Watering time (ms): <input type=\"number\" id=\"watering_time\" required></label><br>"
"  <label>Sample count: <input type=\"number\" id=\"sample_count\" required></label><br>"
"  <label>Read delay (ms): <input type=\"number\" id=\"read_delay\" required></label><br>"
"  <label>Dry threshold (%): <input type=\"number\" step=\"0.1\" id=\"dry_threshold\" required></label><br>"
"  <button type=\"submit\">Zmien konfiguracje</button>"
"</form>"
"<p id=\"configResult\"></p>"

"<h2>Aktualna konfiguracja:</h2>"
"<pre id=\"currentConfig\">Ładowanie...</pre>"

"<script>"
"function fetchHumidity() {"
"  fetch('/api/humidity')"
"    .then(response => response.json())"
"    .then(data => {"
"      document.getElementById('humidity').innerText = data.humidity_percent + ' %';"
"    })"
"    .catch(() => {"
"      document.getElementById('humidity').innerText = 'Blad odczytu';"
"    });"
"}"

"function fetchCurrentConfig() {"
"  fetch('/api/config')"
"    .then(response => response.json())"
"    .then(data => {"
"      document.getElementById('currentConfig').innerText = JSON.stringify(data, null, 2);"
"      document.getElementById('watering_time').value = data.watering_time;"
"      document.getElementById('sample_count').value = data.sample_count;"
"      document.getElementById('read_delay').value = data.read_delay;"
"      document.getElementById('dry_threshold').value = data.dry_threshold;"
"    })"
"    .catch(() => {"
"      document.getElementById('currentConfig').innerText = 'Blad odczytu konfiguracji';"
"    });"
"}"

"document.getElementById('waterBtn').addEventListener('click', function() {"
"  fetch('/api/water', { method: 'POST' })"
"    .then(response => response.json())"
"    .then(data => {"
"      document.getElementById('waterResult').innerText = data.message || 'Podlewanie rozpoczęte';"
"    })"
"    .catch(() => {"
"      document.getElementById('waterResult').innerText = 'Blad podlewania';"
"    });"
"});"

"document.getElementById('configForm').addEventListener('submit', function(e) {"
"  e.preventDefault();"
"  const config = {"
"    watering_time: parseInt(document.getElementById('watering_time').value),"
"    sample_count: parseInt(document.getElementById('sample_count').value),"
"    read_delay: parseInt(document.getElementById('read_delay').value),"
"    dry_threshold: parseFloat(document.getElementById('dry_threshold').value)"
"  };"
"  fetch('/api/config', {"
"    method: 'POST',"
"    headers: { 'Content-Type': 'application/json' },"
"    body: JSON.stringify(config)"
"  })"
"  .then(response => response.text())"
"  .then(msg => {"
"    document.getElementById('configResult').innerText = 'Konfiguracja zapisana: ' + msg;"
"    fetchCurrentConfig();"
"  })"
"  .catch(() => {"
"    document.getElementById('configResult').innerText = 'Blad zapisu konfiguracji';"
"  });"
"});"

"setInterval(fetchHumidity, 1000);"
"fetchHumidity();"
"fetchCurrentConfig();"
"</script>"
"</body></html>";


	// Ustawienie nagłówka odpowiedzi (Content-Type) na html
    httpd_resp_set_type(req, "text/html");
    
    // Wysłanie odpowiedzi
    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
    
    return ESP_OK;
}

esp_err_t water_post_handler(httpd_req_t *req) 
{
	uint32_t watering_signal = 1;  // Sygnał podlewania, 1 - podlej
	
    // Przez 100ms próbuje dodać sygnał podlewania do kolejki, jeśli kolejka będzie
    // zajęta przed dłużej niż 100 ms - zwróci błąd 503.
    if (xQueueSend(watering_queue, &watering_signal, pdMS_TO_TICKS(100)) != pdPASS) {
	    // Kolejka pełna – odpowiedz błędem
	    httpd_resp_set_status(req, "503 Service Unavailable");
	    httpd_resp_sendstr(req, "{\"error\":\"Nie można dodać zadania podlewania.\"}");
	    return ESP_OK;
	}
    
    // Wyślij odpowiedź, status 200 OK
	httpd_resp_set_type(req, "application/json");
	httpd_resp_set_status(req, "200 OK");
	const char *resp = "{\"message\":\"Podlewanie rozpoczete.\"}";
    httpd_resp_sendstr(req, resp);
    
	return ESP_OK;
}

esp_err_t humidity_get_handler(httpd_req_t *req)
{
	// Inicjalizacja obiektu JSON
	cJSON *json = cJSON_CreateObject();
    if (!json) return ESP_FAIL;
    
    float humidity = 0.0f;
    
    // Odczyt aktualnej wilgotnosci z kolejki
	// Przez 100ms próbuje wyciągnąć aktualną wilgotność z kolejki, jeśli nie wyciągnie - zwróci błąd 409.
	if (xQueueReceive(humidity_queue, &humidity, pdMS_TO_TICKS(100)) != pdTRUE) {
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

  	// Pobierz pola z JSON'a
    cJSON *watering_time_item = cJSON_GetObjectItem(json, "watering_time");
    cJSON *sample_count_item = cJSON_GetObjectItem(json, "sample_count");
    cJSON *read_delay_item = cJSON_GetObjectItem(json, "read_delay");
    cJSON *dry_threshold_item = cJSON_GetObjectItem(json, "dry_threshold");

    // Walidacja typów i zakresów
    if (   !cJSON_IsNumber(watering_time_item) 
    	|| watering_time_item->valueint < MIN_WATERING_TIME 
    	|| watering_time_item->valueint > MAX_WATERING_TIME 
    	
		|| !cJSON_IsNumber(sample_count_item)  
		|| sample_count_item->valueint  < MIN_SAMPLE_COUNT  
		|| sample_count_item->valueint  > MAX_SAMPLE_COUNT 
		
		|| !cJSON_IsNumber(read_delay_item)    
		|| read_delay_item->valueint 	< MIN_READ_DELAY    
		|| read_delay_item->valueint 	> MAX_READ_DELAY 
		
        || !cJSON_IsNumber(dry_threshold_item) 
        || dry_threshold_item->valueint < MIN_DRY_THRESHOLD 
        || dry_threshold_item->valueint > MAX_DRY_THRESHOLD) 
    {
        cJSON_Delete(json);
        httpd_resp_set_status(req, "400 Bad Request");
        httpd_resp_set_type(req, "application/json");
        httpd_resp_sendstr(req, "{\"error\":\"Nieprawidlowe dane w konfiguracji - przekroczono zakres\"}");
        return ESP_OK;
    }

    // Przypisz do user_config
    user_config.watering_time = watering_time_item->valueint;
    user_config.sample_count = sample_count_item->valueint;
    user_config.read_delay = read_delay_item->valueint;
    user_config.dry_threshold = (int)dry_threshold_item->valuedouble;

	// Zapis nowej konfiguracji do pamięci nvs
    config_save(&user_config);
    
    // Zwolnienienie zasobów
    cJSON_Delete(json);

	// Wysłanie odpowiedzi
    httpd_resp_sendstr(req, "OK");
    
    return ESP_OK;
}

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