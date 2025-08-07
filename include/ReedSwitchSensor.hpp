#ifndef REEDSWITCHSENSOR_HPP
#define REEDSWITCHSENSOR_HPP

#include "TrainController.hpp"
#include "Action/SensorAction.hpp"
#include "Sensor.hpp"
#include <memory>

class ActionController; // Forward declaration

class ReedSwitchSensor : public Sensor {
private:
    int pin; // Pin number where the reed switch is connected
    bool currentState; // Current state of the reed switch
    bool lastState; // Last state of the reed switch
    bool trainDetected; // Whether a train is currently detected
    unsigned long lastDebounceTime; // Last time the state was changed
    unsigned long debounceDelay; // Debounce delay in milliseconds
    unsigned long timeout; // Timeout to prevent multiple triggers
    unsigned long timeoutThreshold; // Timeout threshold in milliseconds
    SensorLocation location; // Location of the sensor
    std::unique_ptr<SensorAction> action; // Action to execute when train is detected

    bool readPin(); // Read the current pin state
    bool isStateStable(); // Check if the state is stable after debouncing
public:
    ReedSwitchSensor(int pin, SensorLocation loc, std::unique_ptr<SensorAction> sensorAction);
    bool detectPassingTrain() override; // Main detection method
    bool isTrainDetected() const override; // Check if train is currently detected
    void reset() override; // Reset the sensor state
    void executeAction(TrainController& controller, ActionController& actionController) override; // Execute the associated action
    SensorLocation getLocation() const;
};

#endif // REEDSWITCHSENSOR_HPP