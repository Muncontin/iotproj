# Multi-Protocol Mesh Network for ESP32 (LoRa + BLE + Wi-Fi)

## Overview
This project implements a multi-protocol mesh network tailored for ESP32-based devices such as the TTGO T-Beam. It leverages a combination of communication technologies—LoRa, Bluetooth Low Energy (BLE), and Wi-Fi Mesh (via PainlessMesh)—to build a robust, adaptable network for IoT use cases.

Each node operates in a defined role (`meshMode`), enabling only the protocols required for its function.

---

## Supported Roles (`meshMode`)
You can configure each node with one of the following modes:

| Mode | Description                      |
|------|----------------------------------|
| 0    | LoRa only                        |
| 1    | LoRa + MQTT                      |
| 2    | LoRa + BLE                       |
| 3    | LoRa + BLE + MQTT               |
| 4    | LoRa + BLE + Wi-Fi Mesh (PainlessMesh) |

---

## Project Structure
```
├── src/                        # Main application logic
├── lib/
│   └── LoRaMesher/            # Custom LoRa mesh routing library
├── platformio.ini             # PlatformIO configuration
├── .gitignore                 # Excludes all but LoRaMesher in libdeps
└── README.md
```

---

## Features
- Role-based runtime configuration using `meshMode`
- LoRa mesh routing using modified LoRaMesher
- BLE Server support for local configuration and diagnostics
- MQTT client integration for gateway nodes
- Wi-Fi Mesh support for high-speed local relay
- HELLO packets carry broker IP and role metadata

---

## Getting Started

### 1. Clone the Repository
```bash
git clone https://github.com/your-username/multi-protocol-mesh.git
cd multi-protocol-mesh
```

### 2. Configure the Node Role
Edit `include/config.h`:
```cpp
#define MESH_MODE 3  // Example: LoRa + BLE + MQTT
```

### 3. Build and Upload
```bash
pio run --target upload
```

### 4. Monitor Serial Output
```bash
pio device monitor
```

---

## Contact
For questions, feature requests, or bug reports, please open an issue or start a discussion on the repository.

---

Built using ESP32, LoRa, BLE, and mesh networking.