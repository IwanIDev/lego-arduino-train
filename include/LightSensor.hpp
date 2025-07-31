#ifndef LIGHTSENSOR_HPP
#define LIGHTSENSOR_HPP

#define BUFFER_SIZE 64
#include "SensorAction.hpp"

class LightSensor {
private:
  int pin;
  int threshold;
  int lastReading;
  bool trainDetected;
  unsigned long timeout;
  const unsigned long timeoutThreshold;
  int lightBuffer[BUFFER_SIZE];
  int lightBufferIndex;
  bool bufferFull;
  int lastAverage;
  SensorLocation location;

  int getAverageLightLevel();
  int getDynamicThreshold(int average);

public:
  LightSensor(int sensorPin, int detectionThreshold, SensorLocation loc)
      : pin(sensorPin),
        threshold(detectionThreshold),
        lastReading(0),
        trainDetected(false),
        timeout(0),
        timeoutThreshold(1000), // Set a default value or pass as parameter if needed
        lightBuffer{0},
        lightBufferIndex(0),
        bufferFull(false),
        lastAverage(0),
        location(loc) {}

  int readLevel();
  bool isTrainPassingOver(int lightReading);
  bool detectPassingTrain();

  bool isTrainDetected() const;
  void reset();
  SensorLocation getLocation() const { return location; }
};

#endif // LIGHTSENSOR_HPP
