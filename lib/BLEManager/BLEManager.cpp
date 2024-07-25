#include "BLEManager.h"


BLEManager::BLEManager()
    : pServer(nullptr), pWifiCharacteristic(nullptr),deviceConnected(false), oldDeviceConnected(false) {} // Khởi tạo giá trị mặc định cho các biến

void BLEManager::begin() {
    Serial.begin(115200);

    BLEDevice::init("VGE_1"); // phần khởi tạo buffer cho BLE
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks(*this));

    BLEService* pService = pServer->createService(SERVICE_UUID);


    pWifiCharacteristic = pService->createCharacteristic(
        WIFI_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    pWifiCharacteristic->setCallbacks(new CharacteristicCallbacks(*this));

    pService->start();
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);  // Giá trị không quảng bá
    BLEDevice::startAdvertising();
    Serial.println("Waiting a client connection to notify...");
}

void BLEManager::handleClient() {
    if (deviceConnected) {
        Serial.print("New value notified: ");
        delay(3000);
    }
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
        Serial.println("Device Connected");
    }
}

void BLEManager::disconnectClient() {
    if (pServer) {
        pServer->disconnect(0);  // Ngắt kết nối client đầu tiên
        Serial.println("BLE client disconnected");
    }
}

