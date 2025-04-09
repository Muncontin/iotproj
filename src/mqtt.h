#include <globals.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

WiFiClient espClient;
PubSubClient client(espClient);


const char* willTopic = "network_status";
const char* willMessage = "offline";
const int willQoS = 1;
const bool willRetain = true;


void connectMQTT() {
    if (!client.connected()) {
        Serial.print("Attempting MQTT connection... ");

        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi not connected.");
            return;
        }

        if (client.connect(mqttDeviceName, mqttUser, mqttPassword, willTopic, willQoS, willRetain, willMessage)) { //set last will message
            Serial.println("connected!");
            client.subscribe("from_dashboard");
            meshBrokerIP.fromString(mqttServer);
        } else {
            Serial.print("failed, rc=");
            Serial.println(client.state());
            meshBrokerIP.fromString("0.0.0.0"); // If no broker detected/wifi fail, reset the broker IP address
        }
    }
}





// Callback function to handle received MQTT messages
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("]: ");

    Serial.printf("Received message from dashboard.");
    // Print the message for debug
    String message;
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
        message += (char)payload[i];
    }
    Serial.println();

    // Sanity check: is it for this node?
    StaticJsonDocument<128> doc;
    DeserializationError err = deserializeJson(doc, message);
    if (err) {
        Serial.println("Invalid JSON payload.");
        return;
    }

    String targetStr = doc["address"];  // "3ea0" format
    uint16_t targetAddr = (uint16_t)strtol(targetStr.c_str(), nullptr, 16);

    if (targetAddr == radio.getLocalAddress()) {
        Serial.println("Message for this node, sending to relevant end device.");
    
        // Build message in the same format as "from_dashboard"
        StaticJsonDocument<192> outDoc;
    
        outDoc["address"] = doc["address"];               // Keep same address
        outDoc["end_device_type"] = 0;                    // Set to 0 (broker)
        outDoc["device_id"] = "node-01";                  // Must be included
        outDoc["payload"] = doc["payload"];               // Forward the payload
        outDoc["time"] = "2025-04-09T19:24:08.229Z";      // Ideally get real time, for now use static
    
        char jsonOut[192];
        serializeJson(outDoc, jsonOut);
    
        bool success = client.publish("to_dashboard", jsonOut, true);
        Serial.printf("[MQTT] Published data to 'to_dashboard': %s\n", success ? "success" : "failed");
        return;
    }
    

    // Send raw string over LoRa
    if (length < 192) {
        radio.createPacketAndSend(targetAddr, payload, length);
        Serial.println("Forwarded full MQTT payload over LoRa.");
    } else {
        Serial.println("Payload too big for LoRa packet.");
    }
}


void mqttSetup() {
    //Serial.begin(115200);


    // Connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");



    // Setup MQTT
    client.setServer(mqttServer, mqttPort);
    client.setBufferSize(1028); // Set maximum packet size to 10228 bytes
    client.setKeepAlive(60);
    client.setCallback(callback);
    connectMQTT();
}

void reconnect() {
    while (!client.connected()) {
        Serial.println("[MQTT] Attempting to connect to broker...");

        if (client.connect(mqttDeviceName, mqttUser, mqttPassword, willTopic, willQoS, willRetain, willMessage)) { //set last will message
            Serial.println("[MQTT] Connected to broker!");

            // Subscribe to topics
            client.subscribe("from_dashboard");
            client.subscribe("from_end_device");

            Serial.println("[MQTT] Subscribed to topics: fromDashboard");
        } else {
            Serial.print("[MQTT] Failed, retrying in 5 seconds. State: ");
            Serial.println(client.state());
            delay(5000);
        }
    }
}

