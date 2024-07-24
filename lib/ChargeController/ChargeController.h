#ifndef CHARGECONTROLLER_H_
#define CHARGECONTROLLER_H_

#include <Pilot.h>
#include <Relay.h>

enum State
{
  Ready,
  Charging,
  Error,
  Overheat
};

struct ActualCurrent
{
  float l1;
  float l2;
  float l3;
};

typedef void (*ChargeControllerEventHandler)();

struct ChargingSettings
{
  // Max current (A) that must never be exceeded
  uint8_t maxCurrent = 16;
};

class ChargeController
{
private:
  ChargingSettings settings;
  State state;
  IPilot *pilot;
  Relay *relay;
  VehicleState vehicleState;
  float currentLimit;
  // ActualCurrent _actualCurrent;
  unsigned long _actualCurrentUpdated;
  unsigned long started;
  ChargeControllerEventHandler vehicleStateChange;
  ChargeControllerEventHandler stateChange;
  int dutyCycle;
  void updateVehicleState();
  void detectOverheat();
  void applyCurrentLimit();
  void closeRelay();
  void openRelay(); 


public:
    ChargeController(IPilot &pilot);

    void setup(ChargingSettings settings);
    void loop();

    void startCharging();
    void stopCharging();

    State getState();
    VehicleState getVehicleState();
    unsigned long getElapsedTime();

    const float maxCurrent() { return settings.maxCurrent; }

    float getCurrentLimit();
    void setCurrentLimit(float currentLimit);

    // const ActualCurrent actualCurrent() { return _actualCurrent; }
    // const unsigned long actualCurrentUpdated() { return _actualCurrentUpdated; }
    // void updateActualCurrent(ActualCurrent actualCurrent);

    float getTemp();

    // Event handlers
    void onVehicleStateChange(ChargeControllerEventHandler handler);
    void onStateChange(ChargeControllerEventHandler handler);  
};



#endif // CHARGECONTROLLER_H_