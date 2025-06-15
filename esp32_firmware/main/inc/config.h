#ifndef CONFIG_H
#define CONFIG_H

/**
 * @file config.h
 * @brief Definicje stałych konfiguracyjnych i parametrów dla projektu.
 *
 * Zawiera definicje pinów GPIO, ustawienia domyślne, zakresy walidacji
 * oraz dodatkowe flagi do testów.
 */

// ------------------------------ KONFIGURACJA OGÓLNA --------------------------------------

/** Flaga domyślna dla obsługi przerwań GPIO */
#define ESP_INTR_FLAG_DEFAULT       		0           	

// ------------------------------ GPIO -----------------------------------------------------

/** Pin GPIO do sterowania pompą */
#define GPIO_PUMP_CONTROL               	32      		

/** Pin GPIO przyjmujący stan z przycisku manualnego podlewania */
#define GPIO_MANUAL_WATERING_BUTTON     	21      		

/** Pin GPIO podpięty do czujnika wilgotności */
#define GPIO_HUM_SENSOR             		34          	

/** Pin danych magistrali I2C (SDA) */
#define GPIO_SDA                    		18          	

/** Pin zegara magistrali I2C (SCL) */
#define GPIO_SCL                    		5          		

/** Pin resetu magistrali I2C (-1 oznacza brak pinu) */
#define GPIO_RST                    		-1       	

// ------------------------------ KONFIGURACJA LCD -----------------------------------------

//#define LCD_PIXEL_CLOCK_HZ        (400 * 1000)

/** Szerokość wyświetlacza LCD w pikselach */
#define LCD_H_RES                   		128             

/** Wysokość wyświetlacza LCD w pikselach */
#define LCD_V_RES                   		32          	

// ------------------------------ DOMYŚLNA KONFIGURACJA ------------------------------------

/** Domyślny czas podlewania w ms */
#define DEFAULT_CONFIG_WATERING_TIME		1000			

/** Domyślna liczba pomiarów wilgotności do uśredniania */
#define DEFAULT_CONFIG_SAMPLE_COUNT         20      		

/** Domyślny czas opóźnienia pomiędzy pomiarami wilgotności w ms */
#define DEFAULT_CONFIG_READ_DELAY          	100     		

/** Domyślny próg wilgotności poniżej którego następuje podlewanie */
#define DEFAULT_CONFIG_DRY_THRESHOLD        50      		

// ------------------------------ WALIDACJA DANYCH -----------------------------------------

/** Minimalny dozwolony czas podlewania w ms */
#define MIN_WATERING_TIME					100				

/** Maksymalny dozwolony czas podlewania w ms */
#define MAX_WATERING_TIME					10000			

/** Minimalna dozwolona liczba pomiarów wilgotności do uśredniania */
#define MIN_SAMPLE_COUNT					1				

/** Maksymalna dozwolona liczba pomiarów wilgotności do uśredniania */
#define MAX_SAMPLE_COUNT					10000			

/** Minimalny dozwolony czas opóźnienia pomiędzy pomiarami wilgotności w ms */
#define MIN_READ_DELAY 						50				

/** Maksymalny dozwolony czas opóźnienia pomiędzy pomiarami wilgotności w ms */
#define MAX_READ_DELAY 						10000			

/** Minimalny próg wilgotności do podlewania */
#define MIN_DRY_THRESHOLD					1				

/** Maksymalny próg wilgotności do podlewania */
#define MAX_DRY_THRESHOLD 					99				

// ------------------------------ DODATKOWE DO TESTÓW --------------------------------------

/** Flaga określająca, czy podlewanie nastąpi poniżej (true) czy powyżej (false) progu wilgotności (do testów) */
#define HUM_THRESHOLD_REVERSE       		true    	

#endif // CONFIG_H