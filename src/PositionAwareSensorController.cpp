#include "PositionAwareSensorController.hpp"
#include <Arduino.h>

PositionAwareSensorController::PositionAwareSensorController(ReedSwitchSensorController* reedController, 
                                                             LightSensorController* lightController)
    : reedSwitchController(reedController), lightSensorController(lightController), 
      lastTriggeredLocation(SensorLocation::UNKNOWN), lastTriggerTime(0) {
}

bool PositionAwareSensorController::checkSensors() {
    unsigned long currentTime = millis();

    if (currentTime - lastTriggerTime > DEBOUNCE_TIME) {
        return false;
    }

    bool sensorTriggered = false;
    
    std::vector<Sensor*> triggeredSensors = getTriggeredSensor();

    for (Sensor* sensor : triggeredSensors) {
        if (!sensor) continue;
        sensorTriggered = true;
        Serial.print("Sensor triggered at location: ");
        Serial.println(static_cast<int>(sensor->getLocation()));
    }

    for (const Sensor* sensor : triggeredSensors) {
        if (!sensor) continue;
        SensorLocation loc = sensor->getLocation();
        if (loc != SensorLocation::UNKNOWN) {
            lastTriggeredLocation = loc;
        }
        lastTriggeredLocation = loc;
        lastTriggerTime = currentTime;
    }

    return sensorTriggered;
}

std::vector<Sensor*> PositionAwareSensorController::getTriggeredSensor() const {
    std::vector<Sensor*> triggeredSensors;
    
    // Add triggered reed switch sensors
    if (reedSwitchController && reedSwitchController->getTriggeredSensors().size() > 0) {
        const auto& reedSensors = reedSwitchController->getTriggeredSensors();
        for (ReedSwitchSensor* reedSensor : reedSensors) {
            triggeredSensors.push_back(static_cast<Sensor*>(reedSensor));
        }
    }
    
    // Add triggered light sensor if any
    if (lightSensorController && lightSensorController->getTriggeredSensor()) {
        triggeredSensors.push_back(static_cast<Sensor*>(lightSensorController->getTriggeredSensor()));
    }
    
    return triggeredSensors;
}

bool PositionAwareSensorController::isTrainDetected() const {
    bool reedDetected = reedSwitchController && reedSwitchController->isTrainPassingOver();
    bool lightDetected = lightSensorController && lightSensorController->isTrainPassingOver();
    return reedDetected || lightDetected;
}
