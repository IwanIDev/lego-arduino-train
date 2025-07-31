#ifndef LIGHTSENSOR_HPP
#define LIGHTSENSOR_HPP

#define BUFFER_SIZE 64
#include "Action/SensorAction.hpp"
#include <memory>

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
  std::unique_ptr<SensorAction> action;

  int getAverageLightLevel();
  int getDynamicThreshold(int average);

public:
  LightSensor(int sensorPin, int detectionThreshold, SensorLocation loc, std::unique_ptr<SensorAction> sensorAction);
      
  int readLevel();
  bool isTrainPassingOver(int lightReading);
  bool detectPassingTrain();

  bool isTrainDetected() const;
  void reset();
  SensorLocation getLocation() const { return location; }

  void executeAction(TrainController& controller);
};

#endif // LIGHTSENSOR_HPP
