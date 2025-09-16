#include <Arduino.h>
#include "ReedSwitchSensor.h"
#include "../Controllers/ActionController.h"
#include "../Actions/SensorAction.h"
#include "../Actions/DelayedAction.h"
#include "../Actions/SequentialAction.h"
#include <memory>

// Constructor
ReedSwitchSensor::ReedSwitchSensor(int pin, SensorLocation loc, std::unique_ptr<SensorAction> sensorAction)
    : pin(pin),
      currentState(false),
      lastState(false),
      trainDetected(false),
      lastDebounceTime(0),
      debounceDelay(25), // Reduced from 50ms to 25ms for faster response
      timeout(0),
      timeoutThreshold(0), // Default timeout threshold of 250 milliseconds
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

void ReedSwitchSensor::executeAction(TrainController& controller, ActionController& actionController) {
    if (!action) {
        Serial.println("No action defined for ReedSwitchSensor - using position-based actions instead.");
        return;
    }

    // Check if we already have sequential actions running to prevent duplicate executions
    if (action->isSequentialAction() && actionController.hasActiveSequentialActions()) {
        Serial.println("SequentialAction already active, skipping duplicate execution");
        return;
    }

    if (action->isDelayedAction()) {
        // For DelayedAction, create a fresh instance and add it to ActionController
        // We know it's a DelayedAction, so we can safely static_cast
        DelayedAction* delayedAction = static_cast<DelayedAction*>(action.get());
        actionController.addDelayedAction(delayedAction->createFresh());
        return;
    }

    // Check if this is a SequentialAction
    if (action->isSequentialAction()) {
        // For SequentialAction, clone it and add to ActionController for managed execution
        SequentialAction* sequentialAction = static_cast<SequentialAction*>(action.get());
        actionController.addSequentialAction(sequentialAction->createFresh());
        return;
    }

    // For all other immediate actions, execute directly
    action->execute(controller, actionController);
}

/**
 * Gets the location of the sensor.
 * @return The location of the sensor.
 */
SensorLocation ReedSwitchSensor::getLocation() const {
    return location;
}
