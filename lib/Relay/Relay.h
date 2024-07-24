#ifndef RELAY_H_
#define RELAY_H_

#include <Arduino.h>


class Relay
{
    private:
        unsigned int _pin;
        unsigned long _delay;
        bool _currentState;
        bool _desiredState;
        unsigned long _lastCalledMillis;

    public:
        Relay(size_t pin);

        void setup(unsigned long delay, int initialState);
        void loop();

        void openImmediately();
        void open();
        void close();
};



#endif // RELAY_H_