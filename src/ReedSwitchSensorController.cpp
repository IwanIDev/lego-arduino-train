#include "ReedSwitchSensorController.hpp"
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
    for (int i = 0; i < sensorCount; i++) {
            if (sensors[i] && sensors[i]->detectPassingTrain()) {
                lastTriggeredSensor = sensors[i];
                return true;
            }
        }
        return false;
}

// Returns the location of the last triggered sensor, or a default value if none was triggered
SensorLocation ReedSwitchSensorController::getTriggeredSensorLocation() const {
    return lastTriggeredSensor ? lastTriggeredSensor->getLocation() : SensorLocation::STATION_STOP;
}

ReedSwitchSensor* ReedSwitchSensorController::getTriggeredSensor() const {
    return lastTriggeredSensor;
}
