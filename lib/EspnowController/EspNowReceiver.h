#ifndef ESPNOWRECEIVER_H
#define ESPNOWRECEIVER_H

#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>

// Cấu trúc dữ liệu để nhận thông tin
typedef struct struct_message {
    int id;
    int analogValue1;
    int analogValue2;
    int analogValue3;
} struct_message;

class EspNowManager {
public:
    EspNowManager();
    void init();
    static void onDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
    static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

private:
    static struct_message myData;
};


#endif // ESPNOWRECEIVER_H
