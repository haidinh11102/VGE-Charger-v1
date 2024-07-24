// =============================================================================
// 
//  Phần relay này không cần thiết lắm nhưng cứ để lại đã khi nào cần thì phát triển
// 
// =============================================================================
#include <Arduino.h>
#include <Relay.h>

Relay::Relay(size_t pin)
{
    this->_pin = pin;
}

void Relay::setup(unsigned long delay, int initialState)
{
    _delay = delay;
    pinMode(_pin, OUTPUT);

    _desiredState = initialState;
    digitalWrite(_pin, initialState);
    _currentState = initialState;
}

void Relay::openImmediately()
{
    _desiredState = 0;
    digitalWrite(_pin, 0);
    _currentState = 0;
}

void Relay::open()
{
    if (_desiredState != 0)
    {
        _desiredState = 0;
        _lastCalledMillis = millis();
    }
}


void Relay::close()
{
    if (_desiredState != 1)
    {
        _desiredState = 1;
        _lastCalledMillis = millis();
    }
}

void Relay::loop()
{
    if (_currentState != _desiredState)
    {
        if (millis() - _lastCalledMillis >= _delay)
        {
            digitalWrite(_pin, _desiredState);
            _currentState = _desiredState;
        }
    }
}
