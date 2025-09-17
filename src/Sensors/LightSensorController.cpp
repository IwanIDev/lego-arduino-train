#include "LightSensorController.h"
#include <Arduino.h>

LightSensorController::LightSensorController()
    : sensorCount(0)
{
    // Initialize the sensors array to nullptr
    for (int i = 0; i < MAX_SENSORS; ++i) {
        sensors[i] = nullptr;
    }
}

void LightSensorController::addSensor(LightSensor* sensor) {
    if (sensorCount >= MAX_SENSORS) {
        Serial.println("Max sensors reached, cannot add more.");
        return;
    }
    sensors[sensorCount++] = sensor; // Add the sensor to the array and increment the count
}

bool LightSensorController::isTrainPassingOver() {
    for (int i = 0; i < sensorCount; i++) {
            if (sensors[i] && sensors[i]->detectPassingTrain()) {
                lastTriggeredSensor = sensors[i];
                return true;
            }
        }
        return false;
}

// Returns the location of the last triggered sensor, or a default value if none was triggered
SensorLocation LightSensorController::getTriggeredSensorLocation() const {
    return lastTriggeredSensor ? lastTriggeredSensor->getLocation() 
                               : SensorLocation::createUnknown();
}

LightSensor* LightSensorController::getTriggeredSensor() const {
    return lastTriggeredSensor;
}
