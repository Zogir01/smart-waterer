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

// strona html klienta webowego
const char *html =
"<!DOCTYPE html>\n"
"<html lang=\"pl\">\n"
"<head>\n"
"  <meta charset=\"UTF-8\" />\n"
"  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\" />\n"
"  <title>Monitor Wilgotności Gleby</title>\n"
"  <style>\n"
"    body {\n"
"      font-family: Arial, sans-serif;\n"
"      background-color: #eef2f3;\n"
"      color: #333;\n"
"      display: flex;\n"
"      align-items: center;\n"
"      justify-content: center;\n"
"      min-height: 100vh;\n"
"      margin: 0;\n"
"    }\n"
"\n"
"    .container {\n"
"      background: white;\n"
"      padding: 2rem 3rem;\n"
"      border-radius: 12px;\n"
"      box-shadow: 0 4px 10px rgba(0, 0, 0, 0.1);\n"
"      max-width: 600px;\n"
"      width: 100%;\n"
"    }\n"
"\n"
"    h1, h2 {\n"
"      text-align: center;\n"
"      margin-bottom: 1rem;\n"
"    }\n"
"\n"
"    .value {\n"
"      font-size: 2.5rem;\n"
"      color: #007bff;\n"
"      text-align: center;\n"
"      margin: 1rem 0;\n"
"    }\n"
"\n"
"    .info, .form-section {\n"
"      margin-top: 1.5rem;\n"
"      font-size: 1rem;\n"
"      line-height: 1.6;\n"
"    }\n"
"\n"
"    .timestamp {\n"
"      text-align: center;\n"
"      font-size: 0.9rem;\n"
"      color: #666;\n"
"      margin-top: 1rem;\n"
"    }\n"
"\n"
"    .alert {\n"
"      margin-top: 1rem;\n"
"      padding: 1rem;\n"
"      background-color: #f8d7da;\n"
"      color: #721c24;\n"
"      border: 1px solid #f5c6cb;\n"
"      border-radius: 8px;\n"
"      text-align: center;\n"
"      font-weight: bold;\n"
"      display: none;\n"
"    }\n"
"\n"
"    label {\n"
"      display: block;\n"
"      margin-top: 1rem;\n"
"    }\n"
"\n"
"    input {\n"
"      width: 100%;\n"
"      padding: 0.5rem;\n"
"      margin-top: 0.3rem;\n"
"      border-radius: 6px;\n"
"      border: 1px solid #ccc;\n"
"    }\n"
"\n"
"    button {\n"
"      margin-top: 1.5rem;\n"
"      padding: 0.7rem 1.2rem;\n"
"      background-color: #007bff;\n"
"      color: white;\n"
"      border: none;\n"
"      border-radius: 6px;\n"
"      cursor: pointer;\n"
"      font-size: 1rem;\n"
"      width: 100%;\n"
"      transition: background-color 0.3s ease;\n"
"    }\n"
"\n"
"    button:hover {\n"
"      background-color: #0056b3;\n"
"    }\n"
"\n"
"    #waterBtn {\n"
"      background-color: #28a745;\n"
"      margin-top: 1rem;\n"
"      width: 100%;\n"
"    }\n"
"\n"
"    #waterBtn:hover {\n"
"      background-color: #1e7e34;\n"
"    }\n"
"\n"
"    .status {\n"
"      text-align: center;\n"
"      margin-top: 1rem;\n"
"      font-weight: bold;\n"
"      color: green;\n"
"    }\n"
"  </style>\n"
"</head>\n"
"<body>\n"
"  <div class=\"container\">\n"
"    <h1>Wilgotność Gleby</h1>\n"
"    <div class=\"value\" id=\"wilgotnosc\">-- %</div>\n"
"\n"
"    <div class=\"alert\" id=\"alert\">Uwaga! Zbyt mała wilgotność gleby!</div>\n"
"\n"
"    <div class=\"info\" id=\"dodatkoweDane\">\n"
"      <p><strong>Próg wilgotności:</strong> -- %</p>\n"
"      <p><strong>Czas podlewania:</strong> -- sek</p>\n"
"      <p><strong>Okres podlewania:</strong> -- sek</p>\n"
"      <p><strong>Liczba próbek:</strong> --</p>\n"
"    </div>\n"
"\n"
"    <div class=\"timestamp\" id=\"czas\">Ładowanie danych...</div>\n"
"\n"
"    <div class=\"form-section\">\n"
"      <h2>Konfiguracja urządzenia</h2>\n"
"      <form id=\"configForm\">\n"
"        <label>⏱️ Czas podlewania (s):\n"
"          <input type=\"number\" id=\"watering_time\" required min=\"0\" />\n"
"        </label>\n"
"        <label>🔁 Liczba próbek:\n"
"          <input type=\"number\" id=\"sample_count\" required min=\"0\" />\n"
"        </label>\n"
"        <label>⏳ Opóźnienie między odczytami (s):\n"
"          <input type=\"number\" id=\"read_delay\" required min=\"0\" />\n"
"        </label>\n"
"        <label>💧 Próg wilgotności (%):\n"
"          <input type=\"number\" id=\"dry_threshold\" required min=\"0\" max=\"100\" />\n"
"        </label>\n"
"        <button type=\"submit\">Zapisz konfigurację</button>\n"
"        <div class=\"status\" id=\"statusMsg\"></div>\n"
"      </form>\n"
"\n"
"      <button id=\"waterBtn\" type=\"button\">Podlej</button>\n"
"      <div class=\"status\" id=\"waterStatusMsg\"></div>\n"
"    </div>\n"
"  </div>\n"
"\n"
"  <script>\n"
"    const wilgotnoscEl = document.getElementById('wilgotnosc');\n"
"    const dodatkoweDaneEl = document.getElementById('dodatkoweDane');\n"
"    const czasEl = document.getElementById('czas');\n"
"    const alertEl = document.getElementById('alert');\n"
"    const form = document.getElementById('configForm');\n"
"    const statusMsg = document.getElementById('statusMsg');\n"
"    const waterBtn = document.getElementById('waterBtn');\n"
"    const waterStatusMsg = document.getElementById('waterStatusMsg');\n"
"    let dryThreshold = null;\n"
"\n"
"    async function pobierzWilgotnosc() {\n"
"      try {\n"
"        const res = await fetch('/api/humidity');\n"
"        const data = await res.json();\n"
"        const wilgotnosc = data.humidity_percent;\n"
"\n"
"        wilgotnoscEl.textContent = wilgotnosc.toFixed(1) + ' %';\n"
"        czasEl.textContent = 'Ostatnia aktualizacja: ' + new Date().toLocaleTimeString();\n"
"\n"
"        if (typeof dryThreshold === 'number' && wilgotnosc < dryThreshold) {\n"
"          alertEl.style.display = 'block';\n"
"        } else {\n"
"          alertEl.style.display = 'none';\n"
"        }\n"
"      } catch (err) {\n"
"        console.error('Błąd podczas pobierania wilgotności:', err);\n"
"        wilgotnoscEl.textContent = '-- %';\n"
"        czasEl.textContent = '';\n"
"        alertEl.style.display = 'none';\n"
"      }\n"
"    }\n"
"\n"
"    async function pobierzKonfiguracje() {\n"
"      try {\n"
"        const res = await fetch('/api/config');\n"
"        const config = await res.json();\n"
"\n"
"        dryThreshold = config.dry_threshold;\n"
"\n"
"        dodatkoweDaneEl.innerHTML = `\n"
"          <p><strong>Próg wilgotności:</strong> ${config.dry_threshold} %</p>\n"
"          <p><strong>Czas podlewania:</strong> ${config.watering_time} sek</p>\n"
"          <p><strong>Okres podlewania:</strong> ${config.read_delay} sek</p>\n"
"          <p><strong>Liczba próbek:</strong> ${config.sample_count}</p>\n"
"        `;\n"
"\n"
"        document.getElementById('watering_time').value = config.watering_time;\n"
"        document.getElementById('sample_count').value = config.sample_count;\n"
"        document.getElementById('read_delay').value = config.read_delay;\n"
"        document.getElementById('dry_threshold').value = config.dry_threshold;\n"
"\n"
"      } catch (err) {\n"
"        console.error('Błąd podczas pobierania konfiguracji:', err);\n"
"        dodatkoweDaneEl.innerHTML = `<p>Błąd podczas pobierania konfiguracji.</p>`;\n"
"      }\n"
"    }\n"
"\n"
"    form.addEventListener('submit', async function (e) {\n"
"      e.preventDefault();\n"
"      statusMsg.textContent = '';\n"
"\n"
"      const watering_time = parseInt(document.getElementById('watering_time').value);\n"
"      const sample_count = parseInt(document.getElementById('sample_count').value);\n"
"      const read_delay = parseInt(document.getElementById('read_delay').value);\n"
"      const dry_threshold = parseInt(document.getElementById('dry_threshold').value);\n"
"\n"
"      if (\n"
"        watering_time < 0 ||\n"
"        sample_count < 0 ||\n"
"        read_delay < 0 ||\n"
"        dry_threshold < 0 || dry_threshold > 100\n"
"      ) {\n"
"        statusMsg.style.color = 'red';\n"
"        statusMsg.textContent = 'Wprowadź tylko wartości dodatnie (wilgotność 0–100%)!';\n"
"        return;\n"
"      }\n"
"\n"
"      const payload = { watering_time, sample_count, read_delay, dry_threshold };\n"
"\n"
"      try {\n"
"        const res = await fetch('/api/config', {\n"
"          method: 'POST',\n"
"          headers: { 'Content-Type': 'application/json' },\n"
"          body: JSON.stringify(payload)\n"
"        });\n"
"\n"
"        if (res.ok) {\n"
"          statusMsg.style.color = 'green';\n"
"          statusMsg.textContent = 'Konfiguracja została zapisana.';\n"
"          pobierzKonfiguracje();\n"
"        } else {\n"
"          throw new Error('Błąd zapisu');\n"
"        }\n"
"      } catch (err) {\n"
"        statusMsg.style.color = 'red';\n"
"        statusMsg.textContent = 'Błąd podczas zapisu konfiguracji.';\n"
"        console.error(err);\n"
"      }\n"
"    });\n"
"\n"
"    waterBtn.addEventListener('click', async () => {\n"
"      waterStatusMsg.textContent = '';\n"
"      try {\n"
"        const res = await fetch('/api/water', { method: 'POST' });\n"
"        if (res.ok) {\n"
"          waterStatusMsg.style.color = 'green';\n"
"          waterStatusMsg.textContent = 'Polecenie podlewania wysłane!';\n"
"        } else {\n"
"          throw new Error('Błąd podczas podlewania');\n"
"        }\n"
"      } catch (err) {\n"
"        waterStatusMsg.style.color = 'red';\n"
"        waterStatusMsg.textContent = 'Nie udało się wysłać polecenia podlewania.';\n"
"        console.error(err);\n"
"      }\n"
"    });\n"
"\n"
"    pobierzKonfiguracje();\n"
"    pobierzWilgotnosc();\n"
"    setInterval(pobierzWilgotnosc, 5000);\n"
"    setInterval(pobierzKonfiguracje,5000);\n"
"  </script>\n"
"</body>\n"
"</html>\n";





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