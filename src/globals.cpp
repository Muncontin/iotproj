#include "globals.h"

LoraMesher& radio = LoraMesher::getInstance();
RoutingTableService routingtableService;
char loraIdStr[16];
char mqttDeviceName[16];
char bleServerName[16];
    

void setupLocalAddress() {
    uint16_t addr = radio.getLocalAddress();
    sprintf(mqttDeviceName, "TTGO-%04X", addr);
    sprintf(bleServerName, "BLE-%04X", addr);  // Optional custom format for BLE name
}


// WiFi Credentials
const char* ssid = "";
const char* password = "";

// MQTT Broker Settings     
const char* mqttServer = "192.168.86.153";  // Your broker IP address 
const int mqttPort = 1883;
const char* mqttUser = ""; // Optional
const char* mqttPassword = ""; // Optional


// Broker IP (0,0,0,0) is default value, indicating NO BROKER DETECTED. If connected, it will update automatically. DO NOT EDIT THIS
IPAddress meshBrokerIP(0, 0, 0, 0);

const char* SERVICE_UUID       = "01234567-0123-4567-89ab-0123456789ab";
const char* MESSAGE_CHAR_UUID  = "01234567-0123-4567-89ab-0123456789cd";

uint8_t BLE_CONN_ID[6]; //Default value for BLE connection ID


//painlessmesh settings
const char* MESH_SSID     = "tootnetwork";
const char* MESH_PASSWORD = "meshpotatoes";
const uint16_t MESH_PORT  = 5555;

//Setting LoRa node roles
NodeRoles meshMode = ROLE_LORA_BLE_MQTT; // 0 = LORA_ONLY, 1 = LORA + MQTT, 2 = LORA + BLE, 3 = LORA + BLE + MQTT, 4 = LORA + BLE + PainlessMesh





// AES ENCRYPTION FOR LORA
const uint8_t AES_KEY[16] = {0x00, 0x01, 0x02, 0x03, 
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F};

void encryptPayload(const uint8_t* input, uint8_t* output, size_t length, const uint8_t key[16], uint8_t iv[16]) {
    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, key, 128);

    size_t offset = 0;
    uint8_t stream_block[16];
    // Encrypt in CTR mode (ciphertext length equals plaintext length)
    mbedtls_aes_crypt_ctr(&aes, length, &offset, iv, stream_block, input, output);
    mbedtls_aes_free(&aes);
}

// Helper function to convert binary data to a hex string.
String toHexString(const uint8_t* data, size_t length) {
    String hex;
    const char hexChars[] = "0123456789ABCDEF";
    for (size_t i = 0; i < length; i++) {
        hex += hexChars[(data[i] >> 4) & 0x0F];
        hex += hexChars[data[i] & 0x0F];
    }
    return hex;
}