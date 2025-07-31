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
  LightSensor(int sensorPin, int detectionThreshold, SensorLocation loc);

  int readLevel();
  bool isTrainPassingOver(int lightReading);
  bool detectPassingTrain();

  bool isTrainDetected() const;
  void reset();
  SensorLocation getLocation() const { return location; }
};

#endif // LIGHTSENSOR_HPP
