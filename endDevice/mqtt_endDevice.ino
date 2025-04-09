#include <M5StickCPlus.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

const char* ssid = "Chocolate";
const char* password = "bunnymagpie0520";
const char* mqtt_server = "192.168.86.153";  // Replace with your broker's IP
const char* mqttUser = "enduser"; // Optional
const char* mqttPassword = "password"; // Optional
char mqttClientId[32];  // Global buffer for dynamic ID

WiFiClient espClient;
PubSubClient client(espClient);

bool ledState = false;

void setupWifi();
void callback(char* topic, byte* payload, unsigned int length);
void reconnect();

void setup() {
    M5.begin();
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(0, 0);
    
    pinMode(10, OUTPUT);

    // Generate unique MQTT client ID using MAC address
    uint64_t chipId = ESP.getEfuseMac();  // 48-bit MAC
    sprintf(mqttClientId, "endNode-%04X", (uint16_t)(chipId & 0xFFFF));
    //strcpy(mqttClientId, "endNodeA");

    M5.Lcd.println("Device ID:");
    M5.Lcd.println(mqttClientId);

    setupWifi();
    setupWifi();

    if (WiFi.status() == WL_CONNECTED) {
        client.setServer(mqtt_server, 1883);
        client.setCallback(callback);
        if (client.connect(mqttClientId, mqttUser, mqttPassword)) {
            client.subscribe("to_dashboard");
            M5.Lcd.println("MQTT Connected!");
        } else {
            M5.Lcd.println("MQTT Failed to connect.");
        }
    } else {
        M5.Lcd.println("Skipping MQTT — no Wi-Fi.");
    }

}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();



    M5.update();
    if (M5.BtnA.wasPressed()) {
        StaticJsonDocument<128> doc;
        doc["address"] = "BROADCAST";
        doc["end_device_type"] = 0;
        doc["device_id"] = "mqttClientId";
        doc["payload"] = "hallo";

        // Add ISO 8601 time if needed (optional in embedded)
        // But if using an RTC or NTP time:
        doc["time"] = "2025-04-07T17:18:00Z";  // Replace with actual time

        char buffer[192];
        serializeJson(doc, buffer);
        client.publish("from_end_device", buffer);

        
        // Display confirmation message
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.println("Sent hallo message");
    }
}

void setupWifi() {
    M5.Lcd.println("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);

    int retries = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        M5.Lcd.print(".");
        retries++;
        if (retries > 20) {  // 10 seconds timeout
            M5.Lcd.println("\nWi-Fi failed to connect.");
            return;
        }
    }

    M5.Lcd.println("\nWi-Fi connected!");
    M5.Lcd.print("IP: ");
    M5.Lcd.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
    if (strcmp(topic, "to_dashboard") == 0) {
        // Convert payload bytes into a String
        String message;
        for (unsigned int i = 0; i < length; i++) {
            message += (char)payload[i];
        }

        // Parse JSON
        StaticJsonDocument<192> doc;
        DeserializationError error = deserializeJson(doc, message);

        if (error) {
            Serial.println("Failed to parse JSON");
            return;
        }

        // Extract only the "payload" field
        String extracted = doc["payload"].as<String>();

        // Display on M5
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextColor(WHITE, BLACK);
        M5.Lcd.setTextSize(2);
        M5.Lcd.println("Payload:");
        M5.Lcd.println(extracted);

        Serial.printf("[MQTT] Payload extracted: %s\n", extracted.c_str());
    }
}



void reconnect() {
    while (!client.connected()) {
        if (client.connect(mqttClientId, mqttUser, mqttPassword)) {
            client.subscribe("to_dashboard");
            M5.Lcd.println("Connected!");
        } else {
            M5.Lcd.print("MQTT failed. rc=");
            M5.Lcd.println(client.state());
            delay(5000);
        }
    }
}