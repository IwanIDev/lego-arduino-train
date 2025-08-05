#ifndef REEDSWITCHSENSOR_HPP
#define REEDSWITCHSENSOR_HPP

#include "TrainController.hpp"

class ReedSwitchSensor {
private:
    int pin; // Pin number where the reed switch is connected
    bool lastState; // Last state of the reed switch
    unsigned long lastDebounceTime; // Last time the state was changed
    const unsigned long debounceDelay; // Debounce delay in milliseconds

    void updateState();
public:
    ReedSwitchSensor(int pin);
    bool isTriggered();
    void reset();
    void executeAction(TrainController& controller);
};

#endif // REEDSWITCHSENSOR_HPP