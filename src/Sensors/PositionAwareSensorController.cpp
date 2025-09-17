#include "PositionAwareSensorController.h"
#include <Arduino.h>

PositionAwareSensorController::PositionAwareSensorController(ReedSwitchSensorController* reedController, 
                                                             LightSensorController* lightController)
    : reedSwitchController(reedController), lightSensorController(lightController), 
      lastTriggeredLocation(SensorLocation::createUnknown()), lastTriggerTime(0) {
}

bool PositionAwareSensorController::checkSensors() {
    unsigned long currentTime = millis();

    if (currentTime - lastTriggerTime <= DEBOUNCE_TIME) {
        return false;
    }

    bool sensorTriggered = false;
    
    // Check sensors and update triggered sensors lists
    if (reedSwitchController) {
        bool reedTriggered = reedSwitchController->isTrainPassingOver();
        if (reedTriggered) {
            sensorTriggered = true;
        }
    }
    
    if (lightSensorController) {
        bool lightTriggered = lightSensorController->isTrainPassingOver();
        if (lightTriggered) {
            sensorTriggered = true;
        }
    }
    
    // Now get the triggered sensors for location tracking
    std::vector<Sensor*> triggeredSensors = getTriggeredSensor();

    for (const Sensor* sensor : triggeredSensors) {
        if (!sensor) continue;
        SensorLocation loc = sensor->getLocation();
        if (!loc.isUnknown()) {
            lastTriggeredLocation = loc;
        }
        lastTriggeredLocation = loc;
        lastTriggerTime = currentTime;
    }

    return sensorTriggered;
}

std::vector<Sensor*> PositionAwareSensorController::getTriggeredSensor() const {
    std::vector<Sensor*> triggeredSensors;
    
    if (reedSwitchController) {
        const auto& reedSensors = reedSwitchController->getTriggeredSensors();
        if (reedSensors.size() > 0) {
            for (ReedSwitchSensor* reedSensor : reedSensors) {
                triggeredSensors.push_back(static_cast<Sensor*>(reedSensor));
            }
        }
    }
    
    // Add triggered light sensor if any - no need to call isTrainPassingOver() again as it was already called in checkSensors()
    if (lightSensorController) {
        if (lightSensorController->getTriggeredSensor()) {
            triggeredSensors.push_back(static_cast<Sensor*>(lightSensorController->getTriggeredSensor()));
        }
    }
    
    return triggeredSensors;
}

bool PositionAwareSensorController::isTrainDetected() const {
    bool reedDetected = reedSwitchController && reedSwitchController->isTrainPassingOver();
    bool lightDetected = lightSensorController && lightSensorController->isTrainPassingOver();
    return reedDetected || lightDetected;
}
