#ifndef ESPNOWRECEIVER_H
#define ESPNOWRECEIVER_H

#include <esp_now.h>
#include <WiFi.h>

typedef struct struct_message {
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

class EspNowReceiver {
public:
  EspNowReceiver();
  void begin();
  static void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
};

#endif // ESPNOWRECEIVER_H
