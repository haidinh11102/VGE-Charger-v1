#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <LittleFS.h>

#include <NetworkManager.h>
#include <MqttController.h>
#include <ChargeController.h>
#include <EspNowReceiver.h>
#include <BLEManager.h>

#include "esp32_secrets.h"

#define LED 2
AsyncWebServer server(80);
DNSServer dnsServer;
NetworkManager networkManager;
Pilot pilot;
ChargeController chargeController(pilot);
MqttController mqttController(networkManager, pilot, chargeController);
EspNowManager espNowReceiver;
BLEManager bleManager;
// ==============================================
// 
//                        SETUP
// 
// ==============================================

void setup() { 
  Serial.begin(115200);

  Serial.println("ArduinoEVSE");
  pinMode(LED, OUTPUT);

  // tạo một file system trên bộ nhớ flash
  if (!LittleFS.begin()) {
    Serial.println("Lỗi khi khởi tạo LittleFS!");
    return;
  }

  networkManager.autoConnect(); 

  // kết nối wifi thành công thì đèn sáng
  digitalWrite(LED, HIGH);
  espNowReceiver.init();

  // kết nối mqtt
  struct MqttSettings mqttSettings;
  strncpy(mqttSettings.host, MQTT_HOST, 254);
  #ifdef MQTT_USERNAME
  strncpy(mqttSettings.username, MQTT_USERNAME, 100);
  #endif
  #ifdef MQTT_PASSWORD
  strncpy(mqttSettings.password, MQTT_PASSWORD, 100);
  #endif
  mqttController.setup(mqttSettings);

}

// ==============================================
// 
//                        LOOP
// 
// ==============================================

void loop() {
  networkManager.loop();
  mqttController.loop();
  chargeController.loop();
  bleManager.handleClient();  // Xử lý các sự kiện BLE liên tục
}

