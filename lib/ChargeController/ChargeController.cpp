#include <Arduino.h>
#include "ChargeController.h"
#include "Pilot.h"

#define PIN_AC_RELAY 21 // chân đo điện áp AC từ sạc
// ==============================================
// bỏ qua so sánh với súng điện
#define LEDC_CHANNEL_0 0
#define LEDC_TIMER_13_BIT 12
#define LEDC_BASE_FREQ 1000 // Frequency 1KHz
// ==============================================


// ==============================================
//
//                updateVehicleState
//
// ==============================================
void ChargeController::updateVehicleState()
{
    VehicleState vehicleState = this->pilot->read();

    if (this->vehicleState != vehicleState)
    {
        this->vehicleState = vehicleState;

        Serial.print("Vehicle state: ");
        char vehicleStateText[50];
        vehicleStateTotext(vehicleState, vehicleStateText);
        Serial.println(vehicleStateText);

        if (vehicleState != VehicleConnected && vehicleState != VehicleReady && vehicleState != VehicleReadyVentilationRequired)
        {
            if (this->state == Charging)
            {
                this->stopCharging();
            }
        }

        this->applyCurrentLimit();

        if (this->vehicleStateChange)
        {
            this->vehicleStateChange();
        }
    }
}

// ==============================================
//
//                ChargeController
//
// ==============================================
ChargeController::ChargeController(IPilot &pilot)
{
    this->pilot = &pilot;
    this->relay = new Relay(PIN_AC_RELAY);
}
// ==============================================
//
//                setup
//
// ==============================================
void ChargeController::setup(ChargingSettings settings)
{
    relay->setup(3000, 0);

    this->pilot->standby();

    this->settings = settings;
    this->currentLimit = this->settings.maxCurrent;
    this->vehicleState = VehicleNotConnected;
    this->state = Ready;
    this->_actualCurrentUpdated = 0;
}
// ==============================================
//
//                loop
//
// ==============================================

void ChargeController::loop()
{
    relay->loop();
    this->updateVehicleState();
}

// ==============================================
//
//                startCharging
//
// ==============================================
void ChargeController::startCharging()
{
    if (this->state == Charging)
    {
        Serial.println("Already charging");
        return;
    }

    // if (vehicleState != VehicleConnected && vehicleState != VehicleReady && vehicleState != VehicleReadyVentilationRequired)
    // {
    //     Serial.println("Vehicle not connected");
    //     return;
    // }

    Serial.println("Start charging");

    this->state = Charging;

    this->started = millis();

    this->applyCurrentLimit();

// ==============================================
// bỏ qua so sánh với súng điện
    // Cấu hình PWM
    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
    ledcAttachPin(PIN_AC_RELAY, LEDC_CHANNEL_0);
    dutyCycle = 2182; // 50% duty cycle
    ledcWrite(LEDC_CHANNEL_0, dutyCycle);
// ==============================================

    if (this->stateChange)
    {
        this->stateChange();
    }
}
// ==============================================
//
//                stopCharging
//
// ==============================================
void ChargeController::stopCharging()
{
    relay->openImmediately(); // immediately, for safety

    if (this->state != Charging)
    {
        Serial.println("Not charging");
        return;
    }

    Serial.println("Stop charging");

    this->state = Ready;

// ==============================================
// bỏ qua so sánh với súng điện
    // Cấu hình PWM
    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
    ledcAttachPin(PIN_AC_RELAY, LEDC_CHANNEL_0);
    dutyCycle = 0; // 0% duty cycle
    ledcWrite(LEDC_CHANNEL_0, dutyCycle);
// ==============================================


    if (this->stateChange)
    {
        this->stateChange();
    }
}

State ChargeController::getState()
{
    return this->state;
}

VehicleState ChargeController::getVehicleState()
{
    return this->vehicleState;
}

float ChargeController::getCurrentLimit()
{
    return this->currentLimit;
}

void ChargeController::setCurrentLimit(float amps)
{
    if (amps < 0)
    {
        amps = 0;
    }

    if (currentLimit != amps)
    {
        this->currentLimit = amps;

        Serial.print("Setting current limit to ");
        Serial.print(amps);
        Serial.println(" A");

        this->applyCurrentLimit();
// ==============================================
// bỏ qua so sánh với súng điện
        // Ghi duty cycle
    // const int dutyCycle = 2000; // 50% duty cycle
        // Cấu hình PWM cho ESP32
        pilot->currentLimit(currentLimit);
        if (amps < MIN_CURRENT)
        {
            amps = MIN_CURRENT;
        }
        else if (amps > MAX_CURRENT)
        {
            amps = MAX_CURRENT;
        }

        // int dutyCycle;

        // Tính duty cycle từ dòng điện
        if (amps <= 51)
        {
            // 6A - 51A
            dutyCycle = 4095 * (amps / 0.6) / 100;        
        }
        else
        {
            // 51A - 80A
            dutyCycle = 4095 * ((amps / 2.5) + 64) / 100;
        }

        Serial.print("Setting pilot duty cycle: ");
        Serial.println(dutyCycle);




    // Cấu hình PWM
        ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
        ledcAttachPin(PIN_AC_RELAY, LEDC_CHANNEL_0);
        ledcWrite(LEDC_CHANNEL_0, dutyCycle);
// ==============================================
    }

    // else if (amps > this->settings.maxCurrent)
    // {
    //     amps = this->settings.maxCurrent;
    // }
}

// void ChargeController::updateActualCurrent(ActualCurrent actualCurrent)
// {
//     Serial.print("Updating actual charging current: ");
//     Serial.print(actualCurrent.l1);
//     Serial.print(" ");
//     Serial.print(actualCurrent.l2);
//     Serial.print(" ");
//     Serial.println(actualCurrent.l3);

//     this->_actualCurrent = actualCurrent;
//     this->_actualCurrentUpdated = millis();
// }

void ChargeController::applyCurrentLimit()
{
    if (this->vehicleState == VehicleConnected || this->vehicleState == VehicleReady || this->vehicleState == VehicleReadyVentilationRequired)
    {
        if (currentLimit >= MIN_CURRENT)
        {
            pilot->currentLimit(currentLimit);

            if (this->state == Charging && (vehicleState == VehicleReady || vehicleState == VehicleReadyVentilationRequired))
            {
                relay->close();
            }
            else
            {
                relay->open();
            }
        }
        else
        {
            relay->open();
            pilot->standby();
        }
    }
    else
    {
        relay->open();
        pilot->standby();
    }

}

unsigned long ChargeController::getElapsedTime()
{
    return millis() - this->started;
}

void ChargeController::onVehicleStateChange(ChargeControllerEventHandler handler)
{
    this->vehicleStateChange = handler;
}

void ChargeController::onStateChange(ChargeControllerEventHandler handler)
{
    this->stateChange = handler;
}
