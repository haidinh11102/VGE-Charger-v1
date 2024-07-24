#include <WiFi.h>
#include <LittleFS.h>


#include "NetworkManager.h"
#include "BLEManager.h"

extern BLEManager bleManager;

unsigned long lastCheckMillis = 0;
int lastStatus = WL_IDLE_STATUS;


// ==============================================
// 
//                      CONNECT
// 
// ==============================================
void NetworkManager::connect()
{
    Serial.print("Connecting to WiFi network: ");
    Serial.println(this->settings.ssid);

    WiFi.disconnect();

    // Attempt to connect to Wifi network:
    lastStatus = WiFi.begin(this->settings.ssid, this->settings.password);
    lastCheckMillis = millis();

    if(WiFi.waitForConnectResult() == WL_CONNECTED) {
        saveWiFiCredentials(); // Lưu cấu hình WiFi khi kết nối thành công
    }

}
// ==============================================
// 
//                      NETWORKMANAGER
// 
// ==============================================
NetworkManager::NetworkManager() : server(80)
{
    client = WiFiClient();
}

// ==============================================
// 
//                 SAVE WIFI CREDENTIALS
// 
// ==============================================
void NetworkManager::saveWiFiCredentials() {
    File file = LittleFS.open("/wifi_credentials4.txt", "w");
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    file.println(this->settings.ssid);
    file.println(this->settings.password);
    file.close();
}

// ==============================================
// 
//                AUTO CONNECT
// 
// ==============================================
void NetworkManager::autoConnect() {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount file system");
        return;
    }

    File file = LittleFS.open("/wifi_credentials4.txt", "r");
  
    String ssid = file.readStringUntil('\n');
    ssid.trim();
    String password = file.readStringUntil('\n');
    password.trim();
    file.close();

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.println("Attempting to connect to WiFi...");

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) {  // Đợi tối đa 10 giây để kết nối
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.print("Successfully connected to ");
        Serial.println(ssid);
        bleManager.disconnectClient();
    } else {
        Serial.println("Failed to connect to WiFi");
        bleManager.begin();
    }
}


// ==============================================
// 
//                      SETUP
// 
// ==============================================
void NetworkManager::setup(WiFiSettings settings)
{
    this->settings = settings;

    // Kiểm tra trạng thái kết nối hiện tại, kiểm tra này chỉ để đảm bảo rằng không có kết nối WiFi nào hiện tại
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Device not connected to any WiFi network.");
    }

    // Đặt tên cho thiết bị để dễ dàng nhận dạng trên mạng
    WiFi.setHostname("ArduinoESP32");

    connect();
}

String getStatusText(int status) {
    switch (status) {
        case WL_IDLE_STATUS:
            return "Not connected";  // Chưa kết nối hoặc đã ngắt kết nối và không cố gắng kết nối lại
        case WL_NO_SSID_AVAIL:
            return "No SSID available";  // Không tìm thấy SSID trong quá trình quét
        case WL_SCAN_COMPLETED:
            return "Scan completed";  // Quét mạng hoàn tất
        case WL_CONNECTED:
            return "Connected";  // Đã kết nối thành công
        case WL_CONNECT_FAILED:
            return "Connect failed";  // Thử kết nối nhưng thất bại
        case WL_CONNECTION_LOST:
            return "Connection lost";  // Mất kết nối sau khi đã kết nối thành công
        case WL_DISCONNECTED:
            return "Disconnected";  // Đã ngắt kết nối từ mạng hiện tại
        default:
            return "Unknown";  // Một trạng thái không xác định hoặc không được liệt kê
    }
}

void NetworkManager::loop() {
    // Kiểm tra khoảng thời gian đã định để cập nhật trạng thái kết nối
    if (millis() - lastCheckMillis >= 2000) {
        lastCheckMillis = millis();
        int status = WiFi.status();

        // Kiểm tra nếu có thay đổi trạng thái kết nối
        if (lastStatus != status) {
            lastStatus = status;
            Serial.print("WiFi status: ");
            Serial.println(getStatusText(status));

            switch (status) {
                case WL_CONNECTED: {
                    IPAddress ip = WiFi.localIP(); // Lấy và hiển thị địa chỉ IP
                    Serial.print("IP address: ");
                    Serial.println(ip);
                    // lấy địa chỉ mac
                    Serial.print("MAC address: ");
                    Serial.println(WiFi.macAddress());
                    Serial.print("Signal strength: "); // Hiển thị cường độ tín hiệu
                    Serial.print(WiFi.RSSI());
                    Serial.println(" dBm");

                    // Gọi hàm xử lý khi đã kết nối
                    if (this->connected) {
                        this->connected();
                    }
                    break;
                }
                case WL_CONNECTION_LOST:
                case WL_DISCONNECTED:
                    // Gọi hàm xử lý khi mất kết nối
                    if (this->disconnected) {
                        this->disconnected();
                    }
                    break;
            }
        }

        // Nếu mất kết nối, thử kết nối lại
        if (status == WL_CONNECTION_LOST || status == WL_DISCONNECTED || status == WL_CONNECT_FAILED) {
            connect();
        }
    }
}



bool NetworkManager::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

WiFiClient& NetworkManager::getClient()
{
    return client;
}

void NetworkManager::onConnected(EventHandler handler)
{
    this->connected = handler;
}

void NetworkManager::onDisconnected(EventHandler handler)
{
    this->disconnected = handler;
}

