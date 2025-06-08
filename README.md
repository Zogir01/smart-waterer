# 🌱 Smart-Waterer – Intelligent Plant Watering System

**Smart-Waterer** is an ESP32-based system designed to automate the watering of potted plants at home. It combines real-time soil moisture monitoring with mobile app control, allowing both automatic and manual watering through a REST API.

---

## 📸 System Architecture

![Component Diagram](./doc/esp32-component-diagram.jpg)

---

## 🎯 Project Goal

The goal of this project is to develop a comprehensive, low-cost, and user-friendly smart watering system that:

- Automatically waters the plant based on real-time soil moisture readings.
- Allows users to configure watering parameters via a mobile app or web interface.
- Supports both manual and scheduled watering for maximum flexibility.

Unlike many existing products that either focus only on monitoring or provide simple timed watering, Smart-Waterer offers an integrated and intelligent solution.

---

## 🧩 Technologies Used

- **ESP32-WROOM-32** – Dual-core 32-bit microcontroller used as the main controller.
- **ESP-IDF** – Official Espressif development framework for writing ESP32 firmware in C/C++.
- **ESP-IDF FreeRTOS** – Real-time operating system for managing multitasking on ESP32.
- **Java** – Programming language used for building the mobile application for configuration and monitoring.
- **REST API** – Exposed by the ESP32 to enable communication with the mobile app over HTTP.
- **Mock API** – Developed for testing and rapid development of the mobile app.
- **Altium Designer** – Used for designing the device's PCB and schematics.

---

## 📁 Repository Structure

```plaintext
SmartPlant/
├── firmware/               # ESP-IDF project
│   └── ...
├── mobile-app/             # Java/Kotlin Android app
│   └── ...
├── hardware/               # Schematics and PCB files (e.g. from Altium)
│   └── ...
├── doc/                    # Documentation and diagrams
│   └── ...				
```

---

## 🚀 Getting Started

### Requirements

- ESP32-WROOM-32 development board
- ESP-IDF environment set up
- ~~Android Studio (for mobile app development)~~
- ~~Altium Designer (for PCB modifications)~~

### Build & Flash (Firmware)

cd firmware
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor

---

## 📱 Features

- Real-time moisture sensing
- Configurable watering logic (via REST API)
- Manual watering trigger via button or ~~app~~
- Task-based FreeRTOS design (multithreaded)
- Persistent configuration storage via NVS

## 📬 Contact

For questions or contributions, feel free to open an issue or pull request!
