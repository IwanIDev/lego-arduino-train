#include <Arduino.h>
#include "ReedSwitchSensor.hpp"

ReedSwitchSensor::ReedSwitchSensor(int pin)
    : pin(pin), lastState(false), lastDebounceTime(0), debounceDelay(50) {
    pinMode(pin, INPUT_PULLUP); // Set the pin as input with pull-up resistor
}

