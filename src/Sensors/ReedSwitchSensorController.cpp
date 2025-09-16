#include "ReedSwitchSensorController.h"
#include <Arduino.h>

ReedSwitchSensorController::ReedSwitchSensorController()
    : sensorCount(0)
{
    // Initialize the sensors array to nullptr
    for (int i = 0; i < MAX_SENSORS; ++i) {
        sensors[i] = nullptr;
    }
}

void ReedSwitchSensorController::addSensor(ReedSwitchSensor* sensor) {
    if (sensorCount >= MAX_SENSORS) {
        Serial.println("Max sensors reached, cannot add more.");
        return;
    }
    sensors[sensorCount++] = sensor; // Add the sensor to the array and increment the count
}

bool ReedSwitchSensorController::isTrainPassingOver() {
    lastTriggeredSensors.clear();
    bool anyTriggered = false;

    for (int i = 0; i < sensorCount; i++) {
        if (sensors[i] && sensors[i]->detectPassingTrain()) {
            lastTriggeredSensors.push_back(sensors[i]);
            anyTriggered = true;
        }
    }
    return anyTriggered;
}

// Returns the location of the last triggered sensor, or a default value if none was triggered
SensorLocation ReedSwitchSensorController::getTriggeredSensorLocation() const {
    ReedSwitchSensor* sensor = lastTriggeredSensors[0];
    return sensor ? sensor->getLocation() : SensorLocation::UNKNOWN;
}

std::vector<ReedSwitchSensor*> ReedSwitchSensorController::getTriggeredSensors() const {
    return lastTriggeredSensors;
}
