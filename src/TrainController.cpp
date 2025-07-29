#include "TrainController.hpp"
#include <Arduino.h>

// Constructor
TrainController::TrainController(byte motorPort)
    : trainState(STOPPED),
        stateChanged(false),
        previousMillis(0),
        speedSwitchInterval(100), // 100ms default interval for speed switching
        port(motorPort)
{}

// Set the train state and mark as changed if different
void TrainController::setState(SPEED newState) {
    if (trainState != newState) {
        trainState = newState;
        stateChanged = true;
    } else {
        stateChanged = false; // No change if the state is the same
    }
}

// Get the current train state
SPEED TrainController::getState() {
    return trainState;
}

// Check if the state has changed since last clear
bool TrainController::hasStateChanged() {
    return stateChanged;
}

// Clear the state changed flag
void TrainController::clearStateChanged() {
    stateChanged = false;
}

// Get the speed value for a given state
int TrainController::getSpeed(SPEED state) {
    switch (state) {
        case FAST: return 30;
        case SLOW: return 15;
        case STOPPED: return 0;
        default: return 0;
    }
}

// Check if enough time has passed to update speed
bool TrainController::canUpdateSpeed() {
    return (millis() - previousMillis) >= speedSwitchInterval;
}

// Update the timer for speed switching
void TrainController::updateSpeedTimer() {
    previousMillis = millis();
}

// Print the current state to Serial
void TrainController::printState() {
    Serial.print("Train state: ");
    switch (trainState) {
        case STOPPED: Serial.println("STOPPED"); break;
        case SLOW: Serial.println("SLOW"); break;
        case FAST: Serial.println("FAST"); break;
        case REVERSE: Serial.println("REVERSE"); break;
        default: Serial.println("UNKNOWN"); break;
    }
}