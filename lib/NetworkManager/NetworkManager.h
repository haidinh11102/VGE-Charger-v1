#ifndef NETWORKMANAGER_H_
#define NETWORKMANAGER_H_

#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncWebServer.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

using EventHandler = void (*)(); // dòng này tạo ra một kiểu dữ liệu mới có tên là EventHandler, kiểu dữ liệu này là một con trỏ hàm không có tham số và không có giá trị trả về


struct WiFiSettings
{
    char ssid[33];
    char password[64];
};

class NetworkManager
{
    private: // các thành phần private chỉ có thể truy cập từ bên trong class
        WiFiSettings settings;
        EventHandler connected;
        EventHandler disconnected;
        AsyncWebServer server;
        void connect();
        //phát wifi
        
        WiFiClient client;
    
    public: // các thành phần public có thể truy cập từ bên ngoài class

        NetworkManager();
        void setup(WiFiSettings settings);
        void loop();

        bool isConnected();
        WiFiClient& getClient();

        // Event handlers
        void onConnected(EventHandler handler);
        void onDisconnected(EventHandler handler);
        void saveWiFiCredentials(); // Hàm lưu thông tin WiFi
        void autoConnect(); // Hàm tự động kết nối WiFi
        
};




#endif // NETWORKMANAGER_H_