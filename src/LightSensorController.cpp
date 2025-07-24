#include "LightSensorController.hpp"
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
            return true; // A train is detected by at least one sensor
        }
    }
    return false; // No train detected by any sensor
}