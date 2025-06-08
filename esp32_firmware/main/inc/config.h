#ifndef CONFIG_H
#define CONFIG_H

// ------------------------------ KONFIGURACJA OGÓLNA --------------------------------------

#define ESP_INTR_FLAG_DEFAULT       	0           // Flaga dla obslugi przerwań

// ------------------------------ GPIO -----------------------------------------------------

#define GPIO_PUMP_CONTROL               32      	// Pin do włączania/wyłączania pompy
#define GPIO_MANUAL_WATERING_BUTTON     21      	// Pin przyjmujacy stan z przycisku
#define GPIO_HUM_SENSOR             	34          // Pin podpięty do czujnika wilgotności
#define GPIO_SDA                    	18          // Pin danych I2C
#define GPIO_SCL                    	5          	// Pin zegara I2C
#define GPIO_RST                    	-1          // Pin reset I2C

// ------------------------------ KONFIGURACJA LCD -----------------------------------------

//#define LCD_PIXEL_CLOCK_HZ        (400 * 1000)
#define LCD_H_RES                   	128         // Szerokość wyświetlacza
#define LCD_V_RES                   	32          // Wysokość wyświetlacza

// ------------------------------ DOMYSLNA KONFIGURACJA ------------------------------------
#define DEFAULT_CONFIG_WATERING_TIME				1000		// Czas podlewania
#define DEFAULT_CONFIG_SAMPLE_COUNT          		20          // Ilość pomiarów wilgotności do uśrednienia
#define DEFAULT_CONFIG_READ_DELAY          			100         // Czas w ms pomiędzy pomiarami wilgotności
#define DEFAULT_CONFIG_DRY_THRESHOLD          		50          // Próg wilgotności dla którego podlewać

// ------------------------------ DODATKOWE DO TESTÓW --------------------------------------
#define HUM_THRESHOLD_REVERSE       				true        // Określa czy podlewanie odbędzie się poniżej progu czy powyżej (do testów)

#endif // CONFIG_H