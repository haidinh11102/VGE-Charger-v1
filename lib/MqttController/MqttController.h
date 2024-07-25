#ifndef MQTTCONTROLLER_H_
#define MQTTCONTROLLER_H_ 

#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>

#include <NetworkManager.h>
#include <ChargeController.h>




enum Message
{
    StopCharging = 0,
    StartCharging = 1,
    SetCurrentLimit = 2,
    ActualCurrent = 3, // phát triển thêm
    MainsMeterValues = 4, // phát triển thêm
};

struct MqttSettings
{
    char host[254];
    uint16_t port = 1883;
    char username[100];
    char password[100];
    char inTopic[100] = "vge";
    char outTopic[100] = "vge";
    uint16_t reconnectInterval = 5000; // kết nối lại sau 5 giây
    uint16_t updateInterval = 5000; // cập nhật sau 5 giây
};

class MqttController
{
    private:
        NetworkManager *networkManager;
        Pilot *pilot;
        ChargeController *chargeController;
        MqttSettings settings;
        MqttClient *mqttClient;
        bool macSent = false;
        bool connectSuccessSent = false;
        unsigned long lastConnect = 0;
        unsigned long lastUpdateSent = 0;
        void connect();
        void reconnectAutomatically();
        void sendPeriodicUpdate();
        void onMessage(int size);
        void processMessage(char *msg);
        void sendConfigWifi();
        void sendUnits();

        // HÀM GỬI MÃ MAC ĐẾN MQTT
        void sendMacAddress();
        String createMacTopic();
        String ChargerTopic();
        String UnitsTopic();
        String WiFiTopic();


    public:
        MqttController(NetworkManager &networkManager,Pilot &pilot, ChargeController &chargeController);

        void setup(MqttSettings settings);
        void loop();

        void sendUpdate();

        bool isConnected();



};


#endif  // MQTTCONTROLLER_H_