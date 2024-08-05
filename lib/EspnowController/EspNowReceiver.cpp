#include "EspNowReceiver.h"

struct_message EspNowManager::myData; // Khởi tạo biến tĩnh

EspNowManager::EspNowManager() {
    // Constructor, có thể thêm các cài đặt khởi tạo ở đây
}

void EspNowManager::init() {
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_send_cb(onDataSent); // Đăng ký hàm callback khi gửi dữ liệu
    esp_now_register_recv_cb(onDataRecv); // Đăng ký hàm callback khi nhận dữ liệu
}

void EspNowManager::onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    memcpy(&myData, incomingData, sizeof(myData));
    Serial.print("Data received from: ");
    for(int i = 0; i < 6; i++) {
        Serial.printf("%02X", mac[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();
    Serial.printf("Data ID %d: %d bytes\n", myData.id, len);
    Serial.printf("Values: %d, %d, %d\n", myData.analogValue1, myData.analogValue2, myData.analogValue3);
}

void EspNowManager::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("Last Packet Send Status: ");
    if (status == ESP_NOW_SEND_SUCCESS) {
        Serial.println("Delivery Success");
    } else {
        Serial.println("Delivery Fail");
    }
}


