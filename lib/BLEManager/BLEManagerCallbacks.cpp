#include <ArduinoJson.h>

#include "BLEManagerCallbacks.h"
#include "BLEManager.h"
#include "NetworkManager.h"

extern NetworkManager networkManager;

ServerCallbacks::ServerCallbacks(BLEManager& manager) : manager(manager) {}

void ServerCallbacks::onConnect(BLEServer* pServer) {
    manager.deviceConnected = true;
}

void ServerCallbacks::onDisconnect(BLEServer* pServer) {
    manager.deviceConnected = false;
}

CharacteristicCallbacks::CharacteristicCallbacks(BLEManager& manager) : manager(manager) {}

void CharacteristicCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
    if (pCharacteristic->getUUID().toString() == WIFI_CHARACTERISTIC_UUID) {
        std::string value = pCharacteristic->getValue();
        if (!value.empty()) {
            Serial.print("Received WiFi data: ");
            Serial.println(value.c_str());


            // Thêm xử lý dữ liệu WiFi ở đây
            StaticJsonDocument<200> doc;

            // Phân tích chuỗi JSON
            DeserializationError error = deserializeJson(doc, value);
            if (error) {
                Serial.print("Failed to parse JSON: ");
                Serial.println(error.c_str());
                return;
            }

            // Lấy SSID và password từ JSON
            const char* ssid = doc["ssid"];
            const char* password = doc["password"];

                WiFiSettings settings;
                strncpy(settings.ssid, ssid, sizeof(settings.ssid));
                strncpy(settings.password, password, sizeof(settings.password));

                // Gọi phương thức setup của NetworkManager để lưu và kết nối WiFi
                networkManager.setup(settings);
                networkManager.saveWiFiCredentials(); // Lưu thông tin WiFi vào bộ nhớ
                // networkManager.autoConnect();
                manager.disconnectClient();
        }

    }
}
