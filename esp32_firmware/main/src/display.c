// // ------------------------------- WŁASNE NAGŁÓWKI -----------------------------------------
// #include "display.h"
// #include "config.h"

// // ------------------------------- BIBLIOTEKI ZEWNĘTRZNE -----------------------------------
// #include "ssd1306.h"

// // ------------------------------- ESP-IDF -------------------------------------------------

// // ------------------------------- ZMIENNE GLOBALNE ----------------------------------------
// static SSD1306_t oledHandle;

// // -----------------------------------------------------------------------------------------

// void configLcd() {
// 	ESP_LOGI("TEST-OLED", "CONFIG_SDA_GPIO=%d", GPIO_SDA);
// 	ESP_LOGI("TEST-OLED", "CONFIG_SCL_GPIO=%d", GPIO_SCL);
// 	ESP_LOGI("TEST-OLED", "CONFIG_RESET_GPIO=%d", GPIO_RST);
// 	i2c_master_init(&oledHandle, GPIO_SDA, GPIO_SCL, GPIO_RST);

// #if CONFIG_FLIP
// 	oledHandle._flip = true;
// 	ESP_LOGW(TAG, "Flip upside down");
// #endif

// 	ESP_LOGI("TEST-OLED", "Panel is %dx%d", LCD_H_RES, LCD_V_RES);
// 	ssd1306_init(&oledHandle, LCD_H_RES, LCD_V_RES);
// 	ssd1306_contrast(&oledHandle, 0xff);
// }

// void display_humidity(float humidity) { 
//     char pvl_str[6]; // pvl - processed value
//     char *pre_value = "Humidity: ";
//     int str_size =  strlen(pre_value) + 1 + 6;
//     char str[str_size];

//     // Konwersja float -> string
//     sprintf(pvl_str, "%.2f%%", humidity);

//     // Kopiowanie stringa
//     strcpy(str, pre_value);

//     // Łączenie stringów
//     strcat(str, pvl_str);

//     ssd1306_clear_screen(&oledHandle, false);
//     ssd1306_display_text(&oledHandle, 1, str, str_size, false);
// }