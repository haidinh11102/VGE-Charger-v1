#ifndef TEMP_SENSOR_H_
#define TEMP_SENSOR_H_ _

#include <Arduino.h>

class ITempSensor
{
public:
  virtual float read() = 0;
};

class TempSensor : public ITempSensor
{
private:
  uint8_t pinNumber;

public:
  TempSensor(uint8_t pinNumber);
  
  float read();
};

#endif // TEMP_SENSOR_H_