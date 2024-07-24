#ifndef BLEMANAGERCALLBACKS_H_
#define BLEMANAGERCALLBACKS_H_

#include <BLEServer.h>
#include <BLECharacteristic.h>



class BLEManager; // Khai báo lớp BLEManager

class ServerCallbacks : public BLEServerCallbacks {
public:
    ServerCallbacks(BLEManager& manager);
    void onConnect(BLEServer* pServer) override;
    void onDisconnect(BLEServer* pServer) override;
private:
    BLEManager& manager;
};

class CharacteristicCallbacks : public BLECharacteristicCallbacks {
public:
    CharacteristicCallbacks(BLEManager& manager);
    void onWrite(BLECharacteristic* pCharacteristic) override;
private:
    BLEManager& manager;
};

#endif // BLEMANAGERCALLBACKS_H_
