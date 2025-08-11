#include "PositionAwareSensorController.hpp"
#include <Arduino.h>

PositionAwareSensorController::PositionAwareSensorController(ReedSwitchSensorController* reedController, 
                                                             LightSensorController* lightController,
                                                             PositionTracker* tracker)
    : reedSwitchController(reedController), lightSensorController(lightController), 
      positionTracker(tracker), lastTriggeredLocation(SensorLocation::UNKNOWN), lastTriggerTime(0) {
}

bool PositionAwareSensorController::checkSensorsAndUpdatePosition() {
    unsigned long currentTime = millis();
    SensorLocation newLocation = SensorLocation::UNKNOWN;
    bool sensorTriggered = false;
    
    // Check reed switch sensors first (they're more reliable)
    if (reedSwitchController && reedSwitchController->isTrainPassingOver()) {
        newLocation = reedSwitchController->getTriggeredSensorLocation();
        sensorTriggered = true;
        Serial.print("Reed switch triggered at location: ");
        Serial.println(static_cast<int>(newLocation));
    }
    // Check light sensors as backup
    else if (lightSensorController && lightSensorController->isTrainPassingOver()) {
        newLocation = lightSensorController->getTriggeredSensorLocation();
        sensorTriggered = true;
        Serial.print("Light sensor triggered at location: ");
        Serial.println(static_cast<int>(newLocation));
    }
    
    // Update position if a sensor was triggered and enough time has passed (debounce)
    if (sensorTriggered && newLocation != SensorLocation::UNKNOWN && 
        (currentTime - lastTriggerTime) > DEBOUNCE_TIME) {
        
        // Only update if the location actually changed
        if (newLocation != lastTriggeredLocation) {
            lastTriggeredLocation = newLocation;
            lastTriggerTime = currentTime;
            
            // Update the position tracker
            if (positionTracker) {
                positionTracker->updatePosition(newLocation);
            }
            
            Serial.print("Position updated to: ");
            Serial.println(static_cast<int>(newLocation));
            return true;
        }
    }
    
    return false;
}

Sensor* PositionAwareSensorController::getTriggeredSensor() const {
    // Return the most recently triggered sensor
    if (reedSwitchController && reedSwitchController->getTriggeredSensor()) {
        return reedSwitchController->getTriggeredSensor();
    }
    if (lightSensorController && lightSensorController->getTriggeredSensor()) {
        return lightSensorController->getTriggeredSensor();
    }
    return nullptr;
}

bool PositionAwareSensorController::isTrainDetected() const {
    bool reedDetected = reedSwitchController && reedSwitchController->isTrainPassingOver();
    bool lightDetected = lightSensorController && lightSensorController->isTrainPassingOver();
    return reedDetected || lightDetected;
}
