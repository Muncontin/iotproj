#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Arduino.h>
#include <globals.h>
   
// Timer variables
unsigned long lastTime = 0;

unsigned long timerDelay = 15000; // Update information every 15 seconds

bool isClientConnected = false;
BLEAdvertising *pAdvertising;

BLECharacteristic messageCharacteristic(MESSAGE_CHAR_UUID, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ);
BLEDescriptor messageDescriptor(BLEUUID((uint16_t)0x2902));

// Setup callbacks onConnect and onDisconnect
class MyServerCallbacks : public BLEServerCallbacks {
  public:
      void onConnect(BLEServer *pServer) override {
          isClientConnected = true;
          BLE_CONN_ID = pServer->getConnId();
          Serial.printf("[BLE] Client connected. ConnID = %d\n", BLE_CONN_ID);
  
          LoraMesher::getInstance().sendHelloPacketNow();  // 👈 Call it directly!
      }
  
      void onDisconnect(BLEServer *pServer) override {
          isClientConnected = false;
          BLE_CONN_ID = 0;
          pAdvertising->start();
          Serial.println("[BLE] Disconnected. Restarted advertising.");
  
          LoraMesher::getInstance().sendHelloPacketNow();  // 👈 Send update again
      }
  };
  
  
  // Callback to handle received messages
  class MessageCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string receivedMsg = pCharacteristic->getValue();
      if (receivedMsg.length() > 0) {
        Serial.print("[RECEIVED] Client Message: ");
        Serial.println(receivedMsg.c_str());
        std::string receivedMsgToShow = "[BLE Server Received]: " + receivedMsg;
      }
    }
  };

  void setupBLEServer() {
    // Create the BLE Device
    BLEDevice::init(bleServerName);

    // Create the BLE Server
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create the BLE Service
    BLEService *bleService = pServer->createService(SERVICE_UUID);
    messageCharacteristic.setCallbacks(new MessageCallbacks());
    bleService->addCharacteristic(&messageCharacteristic);
    messageDescriptor.setValue("Message");
    messageCharacteristic.addDescriptor(&messageDescriptor);

    // Start the service
    bleService->start();

    // Start advertising
    pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();
    Serial.println("[INFO] Currently waiting for a client connection to notify...");
  }