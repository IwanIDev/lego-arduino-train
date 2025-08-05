#include <Arduino.h>
#include "ReedSwitchSensor.hpp"
#include "Action/SensorAction.hpp"
#include <memory>

// Constructor
ReedSwitchSensor::ReedSwitchSensor(int pin, SensorLocation loc, std::unique_ptr<SensorAction> sensorAction)
    : pin(pin),
      currentState(false),
      lastState(false),
      trainDetected(false),
      lastDebounceTime(0),
      debounceDelay(10),
      timeout(0),
      timeoutThreshold(250), // Default timeout threshold of 250 milliseconds
      location(loc),
      action(std::move(sensorAction)) // Use std::move to transfer ownership of the action
{
    pinMode(pin, INPUT_PULLUP); // Set the pin as input with pull-up resistor
}

/**
 * Reads the current state of the reed switch pin.
 * @return True if the switch is activated (LOW due to pull-up), false otherwise.
 */
bool ReedSwitchSensor::readPin() {
    // Reed switch is activated when pin reads LOW (due to INPUT_PULLUP)
    return digitalRead(pin) == LOW;
}

/**
 * Checks if the reed switch state is stable after debouncing.
 * @return True if the state is stable and different from last state.
 */
bool ReedSwitchSensor::isStateStable() {
    bool reading = readPin();
    unsigned long currentTime = millis();
    
    // If the switch changed, update last state and reset timer
    if (reading != lastState) {
        lastDebounceTime = currentTime;
        lastState = reading;
        return false;
    }
    
    // If the reading has been stable for longer than the debounce delay
    if ((currentTime - lastDebounceTime) > debounceDelay) {
        // If the current state has changed from the last confirmed state
        if (reading != currentState) {
            currentState = reading;
            return true; // State change confirmed
        }
    }
    
    return false; // No stable state change
}

/**
 * Detects a passing train using the reed switch with debouncing and timeout logic.
 * @return True when a new train detection occurs, false otherwise.
 */
bool ReedSwitchSensor::detectPassingTrain() {
    bool stateChanged = isStateStable();
    bool currentReading = readPin();
    
    // If the timeout hasn't been reached, we do not detect a train, irrespective of
    // if the reed switch is triggered or not.
    if (millis() - timeout <= timeoutThreshold) {
        return false;
    } else if (timeout != 0) {
        timeout = 0;
    }
    
    // Only process stable state changes for normal operation
    if (!stateChanged) {
        return false;
    }

    bool detected = false;
    
    // Condition 1: If reed switch is activated and train has not been detected before,
    // we set detected to true.
    if (currentState && !trainDetected) {
        trainDetected = true;
        timeout = millis();
        detected = true;
    } 
    // Condition 2: If reed switch is deactivated and train has been detected before,
    // we reset the condition assuming the train has cleared the sensor.
    else if (!currentState && trainDetected) {
        trainDetected = false;
        timeout = millis();
    }

    const char* output = trainDetected ? " --- TRAIN DETECTED (Reed Switch)! ---" : " --- TRAIN CLEARED (Reed Switch)! ---";
    Serial.println(output);

    return detected;
}

/**
 * Checks if a train is currently detected.
 * @return True if a train is detected, false otherwise.
 */
bool ReedSwitchSensor::isTrainDetected() const {
    return trainDetected;
}

/**
 * Resets the sensor state.
 */
void ReedSwitchSensor::reset() {
    trainDetected = false;
    timeout = 0;
    currentState = false;
    lastState = false;
    lastDebounceTime = 0;
}

/**
 * Executes the associated action if one is defined.
 * @param controller The train controller to operate on.
 */
void ReedSwitchSensor::executeAction(TrainController& controller) {
    if (action) {
        action->execute(controller);
    }
}

