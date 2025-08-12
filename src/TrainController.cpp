#include "TrainController.hpp"
#include <Arduino.h>

// Constructor
TrainController::TrainController(byte motorPort)
    : trainState(STOPPED),
        stateChanged(false),
        previousMillis(0),
        speedSwitchInterval(100), // 100ms default interval for speed switching
        port(motorPort),
        speedMultiplier(0) // Initialize speedMultiplier
{}

// Set the train state and mark as changed if different
void TrainController::setState(SPEED newState) {
    if (trainState != newState) {
        trainState = newState;
        stateChanged = true;
    } else {
        stateChanged = false; // No change if the state is the same
    }
    if (newState == STOPPED) {
        speedMultiplier = 0; // Reset multiplier when stopped
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
    int reverseMultiplier = isReverse ? -1 : 1; // Adjust speed for reverse state
    if (speedMultiplier < MIN_MULTIPLIER) {
        speedMultiplier = MIN_MULTIPLIER; // Check for minimum multiplier
    } else if (speedMultiplier > MAX_MULTIPLIER) {
        speedMultiplier = MAX_MULTIPLIER; // Check for maximum multiplier
    }
    // Calculate speed based on state and multiplier
    switch (state) {
        case GO: return 15 * reverseMultiplier * speedMultiplier;
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
        case STOPPED: Serial.print("STOPPED"); break;
        case GO: Serial.print("GO"); break;
        default: Serial.print("UNKNOWN"); break;
    }
    Serial.print(", Reverse: ");
    Serial.print(isReverse ? "ON" : "OFF");
    Serial.print(", Speed Multiplier: ");
    Serial.println(speedMultiplier);
}

void TrainController::incrementSpeed() {
    speedMultiplier = min(speedMultiplier + MULTIPLIER_STEP, MAX_MULTIPLIER);
    if (speedMultiplier > 0) {
        setState(GO);
    }
    Serial.println("Speed incremented");
}

void TrainController::decrementSpeed() {
    speedMultiplier = max(speedMultiplier - MULTIPLIER_STEP, MIN_MULTIPLIER);
    if (speedMultiplier <= 0) {
        setState(STOPPED);
    } else {
        setState(GO);
    }
    Serial.println("Speed decremented");
}