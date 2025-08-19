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
    Serial.print("DEBUG: setState called - old state: ");
    Serial.print(trainState);
    Serial.print(", new state: ");
    Serial.print(newState);
    
    if (trainState != newState) {
        trainState = newState;
        stateChanged = true;
        Serial.println(", state CHANGED, stateChanged set to true");
    } else {
        // Don't clear stateChanged if states are the same - preserve existing flag
        Serial.print(", state SAME, stateChanged remains: ");
        Serial.println(stateChanged);
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
    Serial.println("DEBUG: clearStateChanged() called, setting stateChanged to false");
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
    
    // If speedMultiplier is 0, always return 0 regardless of state
    if (speedMultiplier <= 0) {
        Serial.print("DEBUG: speedMultiplier is 0, returning speed 0");
        return 0;
    }
    
    int calculatedSpeed = 0;
    // Calculate speed based on state and multiplier
    switch (state) {
        case GO: 
            calculatedSpeed = (int)(50 * reverseMultiplier * speedMultiplier);
            break;
        case STOPPED: 
            calculatedSpeed = 0;
            break;
        default: 
            calculatedSpeed = 0;
            break;
    }
    
    Serial.print("DEBUG: getSpeed() - state=");
    Serial.print(state);
    Serial.print(", multiplier=");
    Serial.print(speedMultiplier);
    Serial.print(", reverseMultiplier=");
    Serial.print(reverseMultiplier);
    Serial.print(", calculatedSpeed=");
    Serial.println(calculatedSpeed);
    
    return calculatedSpeed;
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
    float oldMultiplier = speedMultiplier;
    speedMultiplier = min(speedMultiplier + MULTIPLIER_STEP, MAX_MULTIPLIER);
    
    // Force state change if multiplier actually changed
    if (oldMultiplier != speedMultiplier) {
        if (speedMultiplier > 0) {
            setState(GO);
        }
        // Force state change flag even if state is the same
        stateChanged = true;
    }
    
    Serial.print("Speed incremented - speedMultiplier: ");
    Serial.println(speedMultiplier);
}

void TrainController::decrementSpeed() {
    float oldMultiplier = speedMultiplier;
    SPEED oldState = trainState;
    
    speedMultiplier = max(speedMultiplier - MULTIPLIER_STEP, MIN_MULTIPLIER);
    
    // Force state change if multiplier actually changed
    bool multiplierChanged = (oldMultiplier != speedMultiplier);
    
    if (speedMultiplier <= 0) {
        setState(STOPPED);
    } else {
        setState(GO);
    }
    
    // Force state change flag even if state is the same, but only if multiplier changed
    if (multiplierChanged) {
        stateChanged = true;
    }
    
    Serial.print("Speed decremented - oldMultiplier: ");
    Serial.print(oldMultiplier);
    Serial.print(", newMultiplier: ");
    Serial.print(speedMultiplier);
    Serial.print(", oldState: ");
    Serial.print(oldState);
    Serial.print(", newState: ");
    Serial.println(trainState);
}