# Multi-Protocol Mesh Network for ESP32 (LoRa + BLE + Wi-Fi)

## Overview
This project implements a multi-protocol mesh network tailored for ESP32-based devices such as the TTGO T-Beam. It leverages a combination of communication technologies—LoRa, Bluetooth Low Energy (BLE), WiFi communications such as MQTT—to build a robust, adaptable network for IoT use cases.

Each node operates in a defined role (`meshMode`), enabling only the protocols required for its function. Extends LoRaMesher's routing process by including end device information (MQTT broker/BLE clients).

---

## Supported Roles (`meshMode`) [Implementation Incomplete]
You can configure each node with one of the following modes:

| Mode | Description                      |
|------|----------------------------------|
| 0    | LoRa only                        |
| 1    | LoRa + MQTT                      |
| 2    | LoRa + BLE                       |
| 3    | LoRa + BLE + MQTT               |
| 4    | LoRa + BLE + Wi-Fi Mesh (PainlessMesh) [Incomplete] |

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
- BLE client/server support with NimBLE
- MQTT client integration for gateway nodes
- Mesh network is aware of non-LoRa end devices
- Wi-Fi Mesh support for high-speed local relay (Incomplete)

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

## MQTT Dashboard Setup
1. Download [Node-Red](https://nodered.org/docs/getting-started/local) to run locally on the system
```
sudo npm install -g --unsafe-perm node-red
```
2. Install the [FlowFuse library](https://flows.nodered.org/node/@flowfuse/node-red-dashboard) into the node palette

FlowFuse is an additional library of flow nodes that will be used to display MQTT data.
Navigate to the Top right drop down -> Manage palette
![Node-Red Palette Management Screen](https://github.com/user-attachments/assets/0c51c030-6974-4023-92a2-7f987d329ae5)

Download the FlowFuse add-on.

3. Import the flows.json file located in the project /node-red folder
Top right hamburger drop down icon -> Import

4. Start up the Flow Fuse dashboard
On the top right Information section of the dashboard page, click the drop-down arrow and select 'Dashboard 2.0'

![Dashboard_2.0](https://github.com/user-attachments/assets/a4ac198b-86ff-4767-b009-a2969d4aa2f2)

Then click the 'Open Dashboard' button to start the dashboard up

---

## Raspberry Pi AP Point Setup
A basic setup guide has been included: `Setting up raspberry pi broker and AP.pdf`. The guide demonstrates how to set up a Raspberry Pi 4 as a wireless Access Point. 

## Contact
For questions, feature requests, or bug reports, please open an issue or start a discussion on the repository.

---

Built using ESP32, LoRa, BLE, and mesh networking.
