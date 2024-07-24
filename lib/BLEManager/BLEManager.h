#ifndef BLEMANAGER_H_
#define BLEMANAGER_H_

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "BLEManagerCallbacks.h"
#include "NetworkManager.h"

#define SERVICE_UUID        "b03db111-a9ea-4674-b76e-ab7b08c2e885"
#define SENSOR_CHARACTERISTIC_UUID "19b10001-e8f2-537e-4f6c-d104768a1214"
#define WIFI_CHARACTERISTIC_UUID "4d5cb5dd-cc5f-4e1d-a0a8-20c70fd5c266"

class BLEManager {
private:
    BLEServer* pServer;
    BLECharacteristic* pWifiCharacteristic;
    bool deviceConnected;
    bool oldDeviceConnected;

    friend class ServerCallbacks; // Khai báo lớp ServerCallbacks là bạn của lớp BLEManager
    friend class CharacteristicCallbacks; // Khai báo lớp CharacteristicCallbacks là bạn của lớp BLEManager

public:
    BLEManager();
    void begin();
    void handleClient();
    void disconnectClient();
};

#endif // BLEMANAGER_H_
