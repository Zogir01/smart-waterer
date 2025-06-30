# Smart-Waterer – Intelligent Plant Watering System

**Smart-Waterer** is an ESP32-based system designed to automate the watering of potted plant at home. It combines real-time soil moisture monitoring with web control, allowing both automatic and manual watering through a REST API.
Currently, ESP32 works in WiFi AccessPoint mode, but we plan to enable its connection to an existing network (station mode)

---

## System Architecture

![Component Diagram](./doc/esp32-component-diagram.jpg)

The system is built in a real-time operating system (RTOS). It consists of several main elements:
- `taskReadHumiditySensor`: humidity reading from ADC
- `taskMakeDecisionToWater`: automatic watering management
- `taskWaterPump`: support for switching on the pump for a specified time
- `watering_queue`: queue of watering signals read by taskWaterPump
- `humidity_queue`: queue of current humidity readings
- `user_config`: user configuration structure

The http server is configured on the esp32. Following REST endpoints have been implemented:
- `GET /` – download the html+css+js page with the user interface,
- `GET /api/humidity` – download the current humidity,
- `POST /api/water` – manually start watering,
- `GET /api/config` – download the current configuration,
- `POST /api/config` – save the new configuration.

From the webpage we can change user configuration structure, which consists of the following settings:
- `watering_time`: Watering duration in milliseconds.
- `watering_interval`: Minimum time (in milliseconds) between watering cycles.
- `sample_count`: Number of moisture measurements to average.
- `read_delay`: Time in milliseconds between moisture measurements.
- `dry_threshold`: Moisture threshold (%) below which watering is triggered.

The user configuration structure is read at program startup and saved to NVS memory each time the user changes the configuration.

---

## Board Schematic
TO-DO

---

## Technologies Used

- **ESP32-WROOM-32** – Dual-core 32-bit microcontroller used as the main controller.
- **ESP-IDF** – Official Espressif development framework for writing ESP32 firmware in C/C++.
- **ESP-IDF FreeRTOS** – Real-time operating system for managing multitasking on ESP32.
- **html+css+js** – Used to create a website hosted by esp32
- **Altium Designer** – Used for designing the device's PCB and schematics.

---

## Getting Started

### Requirements

- ESP32-WROOM-32 development board
- ESP-IDF environment set up

### Build & Flash (Firmware)

1. cd esp32_firmware 
2. idf.py build 
3. idf.py -p /dev/ttyUSB0 flash monitor

---

## Contact

For questions or contributions, feel free to open an issue or pull request!
