#pragma once
#include <IPAddress.h>
#include "LoraMesher.h"

// Just declare — do not define
extern LoraMesher& radio;
extern RoutingTableService routingtableService;


// WiFi Credentials
extern const char* ssid;
extern const char* password;

// MQTT Broker Settings
extern const char* mqttServer;
extern const int mqttPort;
extern const char* mqttUser;
extern const char* mqttPassword;

// Globally accessible broker IP (default to 0.0.0.0)
extern IPAddress meshBrokerIP;

//BLE
extern const char* SERVICE_UUID;
extern const char* MESSAGE_CHAR_UUID;
extern const char* bleServerName;
extern uint16_t BLE_CONN_ID; // Connection ID for BLE connections

//Painlessmesh
extern const char* MESH_SSID;
extern const char* MESH_PASSWORD;
extern const uint16_t MESH_PORT;


enum NodeRoles {
// 0 = LORA_ONLY
// 1 = LORA + MQTT
// 2 = LORA + BLE
// 3 = LORA + BLE + MQTT
// 4 = LORA + BLE + PainlessMesh

    ROLE_LORA_ONLY = 0,
    ROLE_LORA_MQTT = 1,
    ROLE_LORA_BLE = 2,
    ROLE_LORA_BLE_MQTT = 3,
    ROLE_LORA_BLE_MESH = 4
};
  

extern NodeRoles meshMode; // 0 = LORA_ONLY, 1 = LORA + MQTT, 2 = LORA + BLE, 3 = LORA + BLE + MQTT, 4 = LORA + BLE + PainlessMesh