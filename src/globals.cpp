#include "globals.h"

LoraMesher& radio = LoraMesher::getInstance();
RoutingTableService routingtableService;

// WiFi Credentials
const char* ssid = "Chocolate";
const char* password = "bunnymagpie0520";

// MQTT Broker Settings
const char* mqttServer = "192.168.86.35";  // Your broker IP address 
const int mqttPort = 1883;
const char* mqttUser = ""; // Optional
const char* mqttPassword = ""; // Optional

// Broker IP (0,0,0,0) is default value, indicating NO BROKER DETECTED. If connected, it will update automatically. DO NOT EDIT THIS
IPAddress meshBrokerIP(0, 0, 0, 0);

const char* SERVICE_UUID       = "01234567-0123-4567-89ab-0123456789ab";
const char* MESSAGE_CHAR_UUID  = "01234567-0123-4567-89ab-0123456789cd";
const char* bleServerName      = "IOT-PROJ";
uint16_t BLE_CONN_ID = 0;

//painlessmesh
const char* MESH_SSID     = "tootnetwork";
const char* MESH_PASSWORD = "meshpotatoes";
const uint16_t MESH_PORT  = 5555;

//Setting LoRa node roles
NodeRoles meshMode = ROLE_LORA_BLE_MQTT; // 0 = LORA_ONLY, 1 = LORA + MQTT, 2 = LORA + BLE, 3 = LORA + BLE + MQTT, 4 = LORA + BLE + PainlessMesh