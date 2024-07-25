#include <Arduino.h>
#include <ArduinoMqttClient.h>
#include <MqttController.h>
#include <vector>


#define MAX_MSG_LEN 5000

// ==============================================
// 
//                     connect
// 
// ==============================================

void MqttController::connect()
{
    lastConnect = millis();
    Serial.print("Attempting to connect to the MQTT broker: ");
    Serial.print(settings.host);
    Serial.print(":");
    Serial.println(settings.port);

    mqttClient->stop();

    if (settings.username != NULL)
    {
        mqttClient->setUsernamePassword(settings.username, settings.password);
    }

    mqttClient->setId("ESP32EVSE");
    
    if (!mqttClient->connect(settings.host, settings.port))
    {
        Serial.print("MQTT connection failed! Error code: ");
        Serial.println(mqttClient->connectError());
    }

    Serial.println("Connected to the MQTT broker");


    // Đăng ký nhận tin nhắn từ topic
    String ChargerTopic = this->ChargerTopic();
    mqttClient->subscribe(ChargerTopic);
    Serial.print("Subscribed to ");
    Serial.println(ChargerTopic);


    sendUpdate();

}
// ==============================================
// 
//                 reconnectAutomatically
// 
// ==============================================
void MqttController::reconnectAutomatically()
{
    if (millis() - lastConnect >= settings.reconnectInterval)
    {
        connect();
    }
}

// ==============================================
// 
//                  sendPeriodicUpdate
// 
// ==============================================
void MqttController::sendPeriodicUpdate()
{
    if (millis() - lastUpdateSent >= settings.updateInterval)
    {
        sendConfigWifi();
        sendMacAddress();
        sendUpdate();
        sendUnits();
        
    }

}

// ==============================================
// 
//                    onMessage
// 
// ==============================================
void MqttController::onMessage(int size)
{
    if (size > MAX_MSG_LEN)
    {
        size = MAX_MSG_LEN;
    }

    // char msg[size];

    std::vector<char> msg(size + 1);  // Use a vector to handle variable size
    if (this->mqttClient->read(reinterpret_cast<uint8_t*>(msg.data()), size) > 0) {
        msg[size] = '\0'; // Null terminate the string
        Serial.print("Received message: ");
        Serial.println(msg.data());
        this->processMessage(msg.data());
    }
}

// ==============================================
// 
//                 processMessage
// 
// ============================================== 
void MqttController::processMessage(char *payload)
{
    char *token = strtok(payload, ",");

    Message message = (Message)atoi(token); // Chuyển đổi token thành một số nguyên
    if (message == StartCharging)
    {
        Serial.println("StartCharging message received");
        chargeController->startCharging();
        sendUpdate();
    }
    else if (message == StopCharging)
    {
        Serial.println("StopCharging message received");
        chargeController->stopCharging();
        sendUpdate();
    }
    else if (message == SetCurrentLimit)
    {
        Serial.println("SetCurrentLimit message received");
        token = strtok(NULL, ",");
        float currentLimit = atof(token);
        // loadBalancing->setCurrentLimit(currentLimit);
        chargeController->setCurrentLimit(currentLimit);
        // pilot->currentLimit(currentLimit);
        sendUpdate();
    }
    else if (message == ActualCurrent)
    {
        Serial.println("ActualCurrent message received");
        token = strtok(NULL, ",");
        float actualCurrentL1 = atof(token);
        token = strtok(NULL, ",");
        float actualCurrentL2 = atof(token);
        token = strtok(NULL, ",");
        float actualCurrentL3 = atof(token);
        // chargeController->updateActualCurrent({actualCurrentL1, actualCurrentL2, actualCurrentL3});
    }
    else
    {
        Serial.print("Unknown message: ");
        Serial.println(payload);
    }

}

// ==============================================
// 
//                  MqttController  
// 
// ==============================================
MqttController::MqttController(NetworkManager &networkManager, Pilot &pilot, ChargeController &chargeController)
{
    this->networkManager = &networkManager;
    this->pilot = &pilot;
    this->chargeController = &chargeController;
    this->mqttClient = new MqttClient(networkManager.getClient());
    this->lastConnect = 0;
    this->lastUpdateSent = 0;
}
// ==============================================
// 
//                    Setup
// 
// ==============================================
void MqttController::setup(MqttSettings settings)
{
    this->settings = settings;
}
// ==============================================
// 
//                    Loop
// 
// ==============================================
void MqttController::loop()
{
    if (!mqttClient->connected() || !networkManager->isConnected())
    {
        reconnectAutomatically();
        return;
    }
    int messageSize;
    while (messageSize = mqttClient->parseMessage())
    {
        onMessage(messageSize);
    }

    sendPeriodicUpdate();



}
// ==============================================
// 
//                    sendUpdate
// 
// ==============================================
void MqttController::sendUpdate()
{
    if (!mqttClient->connected())
    {
        return;
    }

    String topic = createMacTopic(); // Gọi hàm để tạo topic mới

    float currentLimit = chargeController->getCurrentLimit();
    float currentLimitFraction = currentLimit - (int)currentLimit;
    int currentLimitDecimals = currentLimitFraction * 10;


    float pilotVoltage = pilot->getVoltage();
    float pilotVoltageFraction = pilotVoltage - (int)pilotVoltage;
    int pilotVoltageDecimals = pilotVoltageFraction * 10;

    char msg[100];
    // test message
    int val1 = 1, val2 = 2, val3 = 3, dec1 = 4, val4 = 5, dec2 = 6, val5 = 7, dec3 = 8;
    sprintf(msg, "%d,%d", chargeController->getState(),
            (int)currentLimit);
// chargeController->getVehicleState() sau khi hoàn thiện thì setup lại
    Serial.print("Sending message to ");
    // Serial.print(settings.outTopic);
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(msg);

    // mqttClient->beginMessage(settings.outTopic);
    mqttClient->beginMessage(topic);
    mqttClient->print(msg);
    mqttClient->endMessage();

    lastUpdateSent = millis();

}
// ==============================================
// 
//                    isConnected
// 
// ==============================================
bool MqttController::isConnected()
{
    return mqttClient->connected();
}


// ==============================================
// 
//                    sendMacAddress
// 
// ==============================================
void MqttController::sendMacAddress()
{
    if (!mqttClient->connected() || macSent)
    {
        return;
    }

    String mac = WiFi.macAddress();
    char macStr[18];  // địa chỉ MAC thường có dạng XX:XX:XX:XX:XX:XX
    strcpy(macStr, mac.c_str());

    Serial.print("Sending MAC address to ");
    Serial.print(settings.outTopic);
    Serial.print(": ");
    Serial.println(macStr);

    mqttClient->beginMessage(settings.outTopic); // Gửi địa chỉ MAC đến topic
    mqttClient->print(macStr);
    mqttClient->endMessage(); // Kết thúc tin nhắn

    macSent = true;
}




// ==============================================
// 
//                    createMacTopic
// 
// ==============================================
String MqttController::createMacTopic() {
    char fullTopic[150]; // Đảm bảo mảng này đủ lớn để chứa toàn bộ chuỗi
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    
    snprintf(fullTopic, sizeof(fullTopic), "%s-%s", settings.outTopic, mac.c_str()); // An toàn hơn khi làm việc với chuỗi

    return String(fullTopic); // Trả về như một đối tượng String

}

// ==============================================
//
//                    ChargerTopic
//
// ==============================================

String MqttController::ChargerTopic() {
    char InTopicCharger[150]; // Đảm bảo mảng này đủ lớn để chứa toàn bộ chuỗi
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    
    snprintf(InTopicCharger, sizeof(InTopicCharger), "%s-%s/charger", settings.inTopic, mac.c_str()); // An toàn hơn khi làm việc với chuỗi

    return String(InTopicCharger); // Trả về như một đối tượng String
}
// ==============================================
//
//                    UnitsTopic
//
// ==============================================

String MqttController::UnitsTopic() {
    char UnitsCharger[150]; // Đảm bảo mảng này đủ lớn để chứa toàn bộ chuỗi
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    
    snprintf(UnitsCharger, sizeof(UnitsCharger), "%s-%s/units", settings.outTopic, mac.c_str()); // An toàn hơn khi làm việc với chuỗi

    return String(UnitsCharger); // Trả về như một đối tượng String
}

// ==============================================
//
//                    wifiTopic
//
// ==============================================

String MqttController::WiFiTopic() {
    char WiFiCharger[150]; // Đảm bảo mảng này đủ lớn để chứa toàn bộ chuỗi
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    
    snprintf(WiFiCharger, sizeof(WiFiCharger), "%s-%s/wifi", settings.outTopic, mac.c_str()); // An toàn hơn khi làm việc với chuỗi

    return String(WiFiCharger); // Trả về như một đối tượng String
}

// ==============================================
//
//                 sendConfigWifi
//
// ==============================================

void MqttController::sendConfigWifi() {

    if (connectSuccessSent) {
        return;
    }

    if (!mqttClient->connected())
    {
        return;
    }

    DynamicJsonDocument doc(1024);
    doc["config_wifi"] = "1";

    String json;
    serializeJson(doc, json);

    String topic = WiFiTopic(); // Gọi hàm để tạo topic mới
    Serial.print("Sending JSON message to ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(json);

    // Bắt đầu gửi message lên topic
    mqttClient->beginMessage(topic);
    mqttClient->print(json);
    mqttClient->endMessage();
    connectSuccessSent = true;
}

// ==============================================
//
//                 sendUnits
//
// ==============================================

void MqttController::sendUnits() {


    if (!mqttClient->connected())
    {
        return;
    }

    DynamicJsonDocument doc(1024);
    float randomKwh = random(0, 100); // Giá trị ngẫu nhiên từ 0.00 đến 99.99
    float randomA = random(0, 50);    // Giá trị ngẫu nhiên từ 0.00 đến 49.99
    float randomV = random(2, 240);   // Giá trị ngẫu nhiên từ 20.0 đến 239.9
    float randomW = random(2, 24);   // Giá trị ngẫu nhiên từ 20.0 đến 239.9
    float randomC = random(2, 100);   // Giá trị ngẫu nhiên từ 20.0 đến 239.9
    doc["kwh"] = randomKwh;  // Giá trị float
    doc["A"] = randomA;    // Giá trị float
    doc["V"] = randomV;   // Giá trị float
    doc["W"] = randomW;   // Giá trị float
    doc["C"] = randomC;    // Giá trị float

    String json;
    serializeJson(doc, json);

    String topic = UnitsTopic(); // Gọi hàm để tạo topic mới
    Serial.print("Sending JSON units to ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(json);

    // Bắt đầu gửi message lên topic
    mqttClient->beginMessage(topic);
    mqttClient->print(json);
    mqttClient->endMessage();
}


