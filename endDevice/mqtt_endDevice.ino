#include <M5StickCPlus.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "your_SSID";  // Replace with your WiFi SSID
const char* password = "your_PASSWORD";  // Replace with your WiFi password
const char* mqtt_server = "192.168.157.160";  // Replace with your broker's IP

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

    pinMode(10, OUTPUT);  // LED Pin
    setupWifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    M5.update();
    if (M5.BtnA.wasPressed()) {
        client.publish("nodeA/led", "toggle");
        
        // Display confirmation message
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.println("Sent LED Toggle to Node A");
    }
}

void setupWifi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    if (strcmp(topic, "nodeB/led") == 0) {
        ledState = !ledState;
        digitalWrite(10, ledState ? HIGH : LOW);
        
        // Display received message
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.println("LED Toggled by Node A");
    }
}

void reconnect() {
    while (!client.connected()) {
        if (client.connect("NodeB")) {
            client.subscribe("nodeB/led");
        } else {
            delay(5000);
        }
    }
}