#ifndef PILOT_H_
#define PILOT_H_

#define MIN_CURRENT 0
#define MAX_CURRENT 80

enum VehicleState
{
    VehicleNotConnected,
    VehicleConnected,
    VehicleReady,
    VehicleReadyVentilationRequired,
    VehicleNoPower,
    VehicleError
};

void vehicleStateTotext(VehicleState vehicleState, char *buffer);

class IPilot
{
public:
  virtual VehicleState read() = 0;
  virtual void standby() = 0;
  virtual void currentLimit(float amps) = 0;
};

class Pilot : public IPilot
{
  float voltage;
  float readPin();

public:
  VehicleState read() override;
  void standby() override;
  void currentLimit(float amps) override;
  float getVoltage();
  Pilot();
};


#endif // PILOT_H_