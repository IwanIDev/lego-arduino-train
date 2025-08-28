#include "TrainController.hpp"
#include "Lpf2Hub.h"
#include <Arduino.h>

// Global map to associate hubs with their TrainController instances
std::map<Lpf2Hub*, TrainController*> hubToControllerMap;

// Constructor
TrainController::TrainController(byte motorPort, Lpf2Hub* trainHub)
    : trainState(STOPPED),
        stateChanged(false),
        previousMillis(0),
        speedSwitchInterval(100), // 100ms default interval for speed switching
        port(motorPort),
        speedMultiplier(0), // Initialize speedMultiplier
        hub(trainHub),
        batteryVoltage(0),
        lastBatteryUpdate(0)
{
    // Register this TrainController instance with its hub
    if (hub) {
        hubToControllerMap[hub] = this;
    }
}

// Destructor
TrainController::~TrainController() {
    // Remove this TrainController instance from the map
    if (hub) {
        hubToControllerMap.erase(hub);
    }
}

// Set the train state and mark as changed if different
void TrainController::setState(SPEED newState) {    
    if (trainState != newState) {
        trainState = newState;
        stateChanged = true;
    } else {
        // Don't clear stateChanged if states are the same - preserve existing flag

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

    // Calculate battery voltage multiplier: 1.0 at 100%, increases inversely as voltage decreases
    float batteryVoltageMultiplier = 1.0f;
    if (batteryVoltage > 0) {
        batteryVoltageMultiplier = 100.0f / batteryVoltage;
        // Cap the maximum multiplier to prevent excessive speed when battery is very low
        if (batteryVoltageMultiplier > 1.5f) {
            batteryVoltageMultiplier = 1.5f;
        }
    }

    int calculatedSpeed = 0;
    // Calculate speed based on state and multiplier
    switch (state) {
        case GO: 
            // Apply speed multiplier to base speed, then apply battery compensation
            calculatedSpeed = (int)(30 * reverseMultiplier * speedMultiplier);
            calculatedSpeed = (int)(calculatedSpeed * batteryVoltageMultiplier);
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

void TrainController::setSpeedMultiplier(float multiplier) {
    if (multiplier < MIN_MULTIPLIER) {
        speedMultiplier = MIN_MULTIPLIER;
    } else if (multiplier > MAX_MULTIPLIER) {
        speedMultiplier = MAX_MULTIPLIER;
    } else {
        speedMultiplier = multiplier;
    }

    if (speedMultiplier <= 0) {
        setState(STOPPED);
    } else {
        setState(GO);
    }
}

float TrainController::getSpeedMultiplier() const {
    return speedMultiplier;
}

// Static callback function for battery voltage updates
void TrainController::batteryVoltageCallback(void* hub, HubPropertyReference property, uint8_t* data) {
    if (property == HubPropertyReference::BATTERY_VOLTAGE) {
        // Use the hub's parseBatteryLevel method to properly extract the battery level
        Lpf2Hub* lpf2Hub = static_cast<Lpf2Hub*>(hub);
        if (lpf2Hub) {
            uint8_t batteryLevel = lpf2Hub->parseBatteryLevel(data);
            Serial.print("Battery voltage received: ");
            Serial.print(batteryLevel);
            Serial.println("%");
            
            // Find the corresponding TrainController instance for this hub
            auto it = hubToControllerMap.find(lpf2Hub);
            if (it != hubToControllerMap.end()) {
                TrainController* controller = it->second;
                if (controller) {
                    controller->batteryVoltage = batteryLevel;
                    controller->lastBatteryUpdate = millis();
                    Serial.print("TrainController battery voltage updated: ");
                    Serial.print(controller->batteryVoltage);
                    Serial.println("%");
                }
            }
        }
    }
}

// Battery voltage methods
uint8_t TrainController::getBatteryVoltage() const {
    return batteryVoltage;
}

void TrainController::updateBatteryVoltage() {
    if (hub && hub->isConnected()) {
        // Request battery voltage update from the hub
        // The callback will directly update this instance's batteryVoltage
        hub->requestHubPropertyUpdate(HubPropertyReference::BATTERY_VOLTAGE, batteryVoltageCallback);
        Serial.println("Requested battery voltage update from hub");
    }
}

bool TrainController::shouldUpdateBatteryVoltage() const {
    return (millis() - lastBatteryUpdate) >= BATTERY_UPDATE_INTERVAL;
}