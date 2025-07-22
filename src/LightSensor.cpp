#include "LightSensor.hpp"
#include <Arduino.h>

// Constructor
LightSensor::LightSensor(int sensorPin, int detectionThreshold)
: pin(sensorPin),
  threshold(detectionThreshold),
  lastReading(0),
  trainDetected(false),
  timeout(0),
  timeoutThreshold(500) // Default timeout threshold of 500 milliseconds
{
  pinMode(pin, INPUT);
}

/**
 * Reads the current light level from the sensor.
 * @return The light level as an integer.
 */
int LightSensor::readLevel() {
  lastReading = analogRead(pin);
  lightBuffer[lightBufferIndex] = lastReading;
  lightBufferIndex = (lightBufferIndex + 1) % BUFFER_SIZE;
  if (lightBufferIndex == 0) {
    bufferFull = true;
  }
  return lastReading;
}

bool LightSensor::isTrainPassingOver(int lightReading) {
  int average = getAverageLightLevel();
  int dynamicThreshold = average * (1 - (threshold / 100.0)); // Adjust threshold based on average light level
  return lightReading < dynamicThreshold;
}

bool LightSensor::detectPassingTrain() {
  int currentLightReading = readLevel();
  bool trainPassing = isTrainPassingOver(currentLightReading);

  // If the timeout hasn't been reached, we do not detect a train, irrespective of
  // if the light sensor is triggered or not.
  if (millis() - timeout <= timeoutThreshold) {
    return false;
  } else if (timeout != 0) {
    timeout = 0;
  }

  // Condition 1: If train is currently passing over and has not been detected before,
  // we set detected to true.
  if (trainPassing && !trainDetected) {
    Serial.println("--- TRAIN DETECTED! ---");
    trainDetected = true;
    timeout = millis();
    return true;
  } 
  // Condition 2: If train has passed over and has been detected before,
  // we reset the condition assuming the train has cleared the sensor.
  else if (!trainPassing && trainDetected) {
    Serial.println("--- TRAIN CLEARED! ---");
    trainDetected = false;
    timeout = millis();
    return false;
  }
  
  return false; // No change in detection state
}

bool LightSensor::isTrainDetected() const {
  return trainDetected;
}

void LightSensor::reset() {
  trainDetected = false;
  timeout = 0;
  lastReading = 0;
}

int LightSensor::getAverageLightLevel() {
  int sum = 0;
  int count = bufferFull ? BUFFER_SIZE : lightBufferIndex;

  for (int i = 0; i < count; i++) {
    sum += lightBuffer[i];
  }

  return (count > 0) ? (sum / count) : 0; // Return average or 0 if no valid readings
}