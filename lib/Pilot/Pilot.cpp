// =============================================================================
// 
//   Đoạn code trong file Pilot.cpp dùng để xác định trạng thái của xe
//   dựa vào điện áp
// =============================================================================
#include <Arduino.h>
#include <math.h>

#include "Pilot.h"

#define LEDC_CHANNEL_0     0
#define LEDC_TIMER_13_BIT  12
#define LEDC_BASE_FREQ     1000 // Frequency 1KHz
#define LED_PIN            21  // Change to GPIO21 for PWM signal

#define PIN_PILOT_IN_MIN_VOLTAGE 2.55 // @ 0V pilot, measured
#define PIN_PILOT_IN_MAX_VOLTAGE 4.47 // @ 12V pilot, measured
#define PIN_PILOT_IN 34 // giá trị này sẽ không được đo ở bên con main chính, mà sẽ được đo ở bên con phụ nhưng cứ viết trước đoạn code vào con main chính này để test 

// ==============================================
//
//                      khoi tao
//
// ==============================================
void PilotInit() {
    // Cấu hình PWM cho ESP32
    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
    ledcAttachPin(LED_PIN, LEDC_CHANNEL_0); // GPIO21
}

Pilot::Pilot()
{
    PilotInit();
}

// ==============================================
// Đọc giá trị điện áp từ chân 
// ==============================================
int analogReadMax(uint8_t pinNumber, uint8_t count)
{
    long result = 0;
    for (int i = 0; i < count; i++)
    {
        result = fmax(result, analogRead(pinNumber));
    }
    return result;
}

// ==============================================
// 
//                     standby
// 
// ==============================================

void Pilot::standby(){
    Serial.println("Setting pilot standby");
    ledcWrite(LEDC_CHANNEL_0, 0);
}
// ==============================================
// 
//                     currentLimit
// 
// ==============================================
void Pilot::currentLimit(float amps){

    if (amps < MIN_CURRENT)
    {
        amps = MIN_CURRENT;
    }
    else if (amps > MAX_CURRENT)
    {
        amps = MAX_CURRENT;
    }

    int dutyCycle;

    // Tính duty cycle từ dòng điện
    if (amps <= 51)
    {
        // 6A - 51A
        dutyCycle = amps / 0.6;        
    }
    else
    {
        // 51A - 80A
        dutyCycle = (amps / 2.5) + 64;
    }

    Serial.print("Setting pilot duty cycle: ");
    Serial.println(dutyCycle);

    PilotInit();
    ledcWrite(LEDC_CHANNEL_0, dutyCycle);
}
// ==============================================
// 
//                     getVoltage
// 
// ==============================================
float Pilot::getVoltage()
{
    return this->voltage;
}

// ==============================================
// 
//                     readPin
// 
// ==============================================
float Pilot::readPin()
{
    // Đọc giá trị điện áp từ chân 
    int pinValue = analogReadMax(PIN_PILOT_IN, 10);
    // Chuyển giá trị ADC thành điện áp
    float pinVoltage = (pinValue / 4095.0) * 5;
    
    this->voltage = ((pinVoltage - PIN_PILOT_IN_MIN_VOLTAGE) / (PIN_PILOT_IN_MAX_VOLTAGE - PIN_PILOT_IN_MIN_VOLTAGE)) * 12;
    return this->voltage;

}

// ==============================================
// 
//                     read
// 
// ==============================================

VehicleState Pilot::read()
{
    float voltage = floor(this->readPin());

    if (voltage >= 11) // 12V +/-1V
    {
        return VehicleNotConnected;
    }
    else if (voltage >= 8) // 9V +/-1V
    {
        return VehicleConnected;
    }
    else if (voltage >= 5) // 6V +/-1V
    {
        return VehicleReady;
    }
    else if (voltage >= 2) // 3V +/-1V
    {
        return VehicleReadyVentilationRequired;
    }
    else if (voltage > 0) // 0V
    {
        return VehicleNoPower;
    }
    else
    {
        return VehicleError; // -12V
    }
}

// ==============================================
// 
//                     vehicleStateTotext
// 
// ==============================================
void vehicleStateTotext(VehicleState vehicleState, char *buffer)
{
    switch (vehicleState)
    {
    case VehicleNotConnected:
        strcpy(buffer, "Not connected"); // 
        break;
    case VehicleConnected:
        strcpy(buffer, "Connected, not ready");
        break;
    case VehicleReady:
        strcpy(buffer, "Ready");
        break;
    case VehicleReadyVentilationRequired:
        strcpy(buffer, "Ready, ventilation required");
        break;
    case VehicleNoPower:
        strcpy(buffer, "No power");
        break;
    case VehicleError:
        strcpy(buffer, "Error");
        break;
    default:
        strcpy(buffer, "Unknown");
        break;
    }
}

// ==============================================