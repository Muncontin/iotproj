#include <Arduino.h>
#include "driver/gpio.h"  // Include the GPIO driver header

#include "ble.h"
#include "mqtt.h"
#include "pmesh.h"

//TODO: dynamically include packages based on the role of the node


//Using LILYGO TTGO T-BEAM v1.2
#define BOARD_LED   4
#define LED_ON      LOW
#define LED_OFF     HIGH

uint32_t dataCounter = 0;
struct dataPacket {
    uint32_t counter = 0;
};

dataPacket* helloPacket = new dataPacket;

//Led flash
void led_Flash(uint16_t flashes, uint16_t delaymS) {
    uint16_t index;
    for (index = 1; index <= flashes; index++) {
        digitalWrite(BOARD_LED, LED_ON);
        delay(delaymS);
        digitalWrite(BOARD_LED, LED_OFF);
        delay(delaymS);
    }
}

// Gets nodes from routing table that are known as ROLE 1 NODES.
// void printGatewayNodes() {
//     NetworkNode* nodes = routingtableService.getAllNetworkNodes();

//     if (!nodes) {
//         Serial.println("Routing table is empty.");
//         return;
//     }

//     int routingSize = routingtableService.routingTableSize();
//     Serial.println("Nodes with Role ID 1:");
//     Serial.println("-----------------------------------------");

//     bool found = false;

//     for (int i = 0; i < routingSize; i++) {
//         if (nodes[i].role == 1) {
//             IPAddress ip = nodes[i].mqttBrokerIP;

//             Serial.printf(
//                 "%d | Address: %04X | Hops: %d | Role: %d | Broker IP: %d.%d.%d.%d\n",
//                 i,
//                 nodes[i].address,
//                 nodes[i].metric,
//                 nodes[i].role,
//                 ip[0], ip[1], ip[2], ip[3]
//             );
//             found = true;
//         }
//     }

//     if (!found) {
//         Serial.println("No nodes with Role ID 1 found.");
//     }

//     delete[] nodes;
// }

void publishRoutingTableToMQTT() {
    // if (!client.connected()) {
    //     Serial.println("[MQTT] Not connected to broker.");
    //     return;
    // }

    NetworkNode* nodes = routingtableService.getAllNetworkNodes();
    if (!nodes) {
        Serial.println("[MQTT] Routing table is empty.");
        return;
    }

    JsonDocument doc;
    //JsonArray table = doc["routing_table"].to<JsonArray>();
    JsonArray table = doc.to<JsonArray>();

    // === Local Node Info ===
    JsonObject self = table.add<JsonObject>();
    self["pos"] = "0";
    self["address"] = String(radio.getLocalAddress(), HEX);
    self["next-hop"] = String(radio.getLocalAddress(), HEX);
    self["hop_count"] = "0";
    self["role"] = "Self";
    self["broker"] = meshBrokerIP.toString();

    bool isBLEValid = false;
    for (int b = 0; b < 6; b++) {
        if (BLE_CONN_ID[b] != 0) {
            isBLEValid = true;
             break;
        }
    }

    if (isBLEValid) {
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                 BLE_CONN_ID[0], BLE_CONN_ID[1], BLE_CONN_ID[2],
                 BLE_CONN_ID[3], BLE_CONN_ID[4], BLE_CONN_ID[5]);
        JsonArray selfBLE = self["BLE Devices"].to<JsonArray>();
        selfBLE.add(macStr);
    }

    // === All Routing Table Nodes ===
    int routingSize = routingtableService.routingTableSize();
    for (int i = 0; i < routingSize; i++) {
        JsonObject node = table.add<JsonObject>();
        node["pos"] = String(i + 1);
        node["address"] = String(nodes[i].address, HEX);
        node["next-hop"] = String(routingtableService.getNextHop(nodes[i].address), HEX);
        node["hop_count"] = String(nodes[i].metric);
        node["role"] = "Role " + String(nodes[i].role);
        node["broker"] = nodes[i].mqttBrokerIP.toString();

        bool valid = false;
        for (int b = 0; b < 6; b++) {
            if (nodes[i].BLE_CONN_ID[b] != 0) {
                valid = true;
                break;
            }
        }

        if (valid) {
            char macStr[18];
            snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                     nodes[i].BLE_CONN_ID[0], nodes[i].BLE_CONN_ID[1], nodes[i].BLE_CONN_ID[2],
                     nodes[i].BLE_CONN_ID[3], nodes[i].BLE_CONN_ID[4], nodes[i].BLE_CONN_ID[5]);

            JsonArray bleList = node["BLE Devices"].to<JsonArray>();
            bleList.add(macStr);
        }
    }

    char buffer[1028];
    size_t len = serializeJson(doc, buffer);
    bool success = client.publish("network_status", buffer, true);

    Serial.printf("[MQTT] Published routing table (%d bytes) to 'network_status': %s\n", len, success ? "success" : "failed");

    delete[] nodes;
}

void sendMessageToRole1Nodes(const String& message) {
    NetworkNode* nodes = routingtableService.getAllNetworkNodes();

    if (!nodes) {
        Serial.println("Routing table is empty.");
        return;
    }

    int routingSize = routingtableService.routingTableSize();
    bool found = false;

    for (int i = 0; i < routingSize; i++) {
        if (nodes[i].role == 1) {
            found = true;
            uint16_t destAddress = nodes[i].address;

            const char* msg = message.c_str();
            size_t msgLen = strlen(msg) + 1;  // Include null terminator

            Serial.printf("Sending to Role 1 node %04X: %s\n", destAddress, msg);

            radio.createPacketAndSend(destAddress, (void*)msg, msgLen);
        }
    }

    if (!found) {
        Serial.println("No Role 1 nodes found to send message.");
    }

    delete[] nodes;
}



void printRoutingTable() {
    routingtableService.printRoutingTable();
}

/**
 * @brief Print the counter of the packet
 *
 * @param data
 */
void printPacket(dataPacket data) {
    Serial.printf("Hello Counter received nº %d\n", data.counter);
}

/**
 * @brief Iterate through the payload of the packet and print the counter of the packet
 *
 * @param packet
 */
void printDataPacket(AppPacket<dataPacket>* packet) {
    Serial.printf("Packet arrived from %X with size %d\n", packet->src, packet->payloadSize);

    //Get the payload to iterate through it
    dataPacket* dPacket = packet->payload;
    size_t payloadLength = packet->getPayloadLength();

    for (size_t i = 0; i < payloadLength; i++) {
        //Print the packet
        printPacket(dPacket[i]);
    }
}

/**
 * @brief Function that process the received packets. Updated to read raw payload
 *
 */
void processReceivedPackets(void*) {
    for (;;) {
        ulTaskNotifyTake(pdPASS, portMAX_DELAY);
        led_Flash(1, 100);

        while (radio.getReceivedQueueSize() > 0) {
            Serial.println("ReceivedUserData_TaskHandle notify received");
            Serial.printf("Queue receiveUserData size: %d\n", radio.getReceivedQueueSize());

            AppPacket<uint8_t>* packet = radio.getNextAppPacket<uint8_t>();

            // Convert payload to string
            String msg;
            for (uint32_t i = 0; i < packet->payloadSize; ++i) {
                msg += (char)packet->payload[i];
            }

            Serial.println("Data received:");
            Serial.println(msg);

            // Parse JSON
            StaticJsonDocument<256> doc;
            DeserializationError err = deserializeJson(doc, msg);
            if (err) {
                Serial.println("Failed to parse JSON in received LoRa message.");
                radio.deletePacket(packet);
                continue;
            }

            // Extract values
            String target = doc["address"] | "";
            int deviceType = doc["end_device_type"] | -1;
            String payload = doc["payload"] | "";

            // Check if this node is the intended recipient
            if (target.equalsIgnoreCase(String(radio.getLocalAddress(), HEX))) {
                if (deviceType == 1) {  // BLE device
                    Serial.printf("Forwarding payload to BLE device: %s\n", payload.c_str());

                    NimBLEService* pSvc = pServer->getServiceByUUID("BAAD");
                    if (pSvc) {
                        NimBLECharacteristic* pChr = pSvc->getCharacteristic("F00D");
                        if (pChr) {
                            pChr->setValue(payload.c_str());
                            pChr->notify();
                        } else {
                            Serial.println("BLE characteristic not found.");
                        }
                    } else {
                        Serial.println("BLE service not found.");
                    }
                } 
                else if (deviceType == 0) {  // MQTT end device
                    if (client.connected()) {
                        bool success = client.publish("to_dashboard", payload.c_str());
                        Serial.printf("Published to MQTT: %s\n", success ? "success" : "failed");
                    } else {
                        Serial.println("MQTT client not connected, can't publish.");
                    }
                } 
                else {
                    Serial.println("Unknown device type, no forwarding action taken.");
                }
            } else {
                Serial.println("Not the intended recipient. Ignoring.");
            }

            printRoutingTable();
            radio.deletePacket(packet);
        }
    }
}





void mqttTask(void* pvParameters) {
    for (;;) {
        if (!client.connected()) {
            reconnect();
        }
        client.loop();
        vTaskDelay(1000 / portTICK_PERIOD_MS);  // Wait 1s
    }
}


TaskHandle_t receiveLoRaMessage_Handle = NULL;

/**
 * @brief Create a Receive Messages Task and add it to the LoRaMesher
 *
 */
void createReceiveMessages() {
    int res = xTaskCreate(
        processReceivedPackets,
        "Receive App Task",
        4096,
        (void*) 1,
        2,
        &receiveLoRaMessage_Handle);
    if (res != pdPASS) {
        Serial.printf("Error: Receive App Task creation gave error: %d\n", res);
    }

    radio.setReceiveAppDataTaskHandle(receiveLoRaMessage_Handle);
}


/**
 * @brief Initialize LoRaMesher
 *
 */
void setupLoraMesher() {
    LoraMesher::LoraMesherConfig config;
    config.module = LoraMesher::LoraModules::SX1262_MOD;

    // Ensure SPI bus is initialized before LoRaMesher begins
    SPI.begin(5, 19, 27, 18); // SCK, MISO, MOSI, NSS

    // Manually specify SPI bus
    config.spi = &SPI;

    // Initialize LoRaMesher
    radio.begin(config);

    RoleService::setRole(meshMode); // Set the role of the node
    createReceiveMessages();

    // Start LoRaMesher
    radio.start();
    Serial.println("LoRa initialized");
}

void setup() {
    Serial.begin(115200);

    // Install the GPIO ISR service
    if (gpio_install_isr_service(0) != ESP_OK) {
        Serial.println("Failed to install GPIO ISR service");
    }

    Serial.println("initBoard");
    pinMode(BOARD_LED, OUTPUT); //setup pin as output for indicator LED
    led_Flash(2, 125);          //two quick LED flashes to indicate program start
    setupLoraMesher();
    setupLocalAddress();

    // Todo: Implement dynamic setups based on what role is selected - Test each scenario (incomplete)
    if (meshMode == ROLE_LORA_BLE) {
        Serial.println("Setting up BLE only");
        setupBLEServer();
    } else if (meshMode == ROLE_LORA_BLE_MQTT) {
        Serial.println("Setting up BLE + MQTT");
        mqttSetup();
        setupBLEServer();

    } else if (meshMode == ROLE_LORA_BLE_MESH) {
        Serial.println("Setting up BLE + PainlessMesh");
        setupBLEServer();
        pMeshSetup();
    } else if (meshMode == ROLE_LORA_MQTT) {
        Serial.println("Setting up BLE + MQTT + PainlessMesh");
        mqttSetup();
    } else if (meshMode == ROLE_LORA_BLE_MESH) {
        Serial.println("Setting up BLE + PainlessMesh");
        pMeshSetup();
    } else if (meshMode == ROLE_LORA_ONLY) {
        Serial.println("Setting up Lora only");
    } else {
        Serial.println("Invalid role selected. Please select a valid role.");
        return;
    }

    xTaskCreate(
        mqttTask,           // Task function
        "MQTT Task",        // Name
        4096,               // Stack size (in words, not bytes)
        NULL,               // Parameter
        1,                  // Priority
        NULL                // Task handle (optional)
    );
    
}


void loop() {
    for (;;) {
        client.loop();
        // Serial.printf("Send packet %d\n", dataCounter);
        // dataCounter++;
        
        // radio.createPacketAndSend(BROADCAST_ADDR, helloPacket, 1);
        //sendMessageToRole1Nodes("This is a directed message to all role 1 nodes");

        RoutingTableService::printRoutingTable();
 
        publishRoutingTableToMQTT();
        bleServerService();

        radio.sendHelloPacketNow();


        //Wait 20 seconds to send the next packet
        vTaskDelay(20000 / portTICK_PERIOD_MS);
    }
}