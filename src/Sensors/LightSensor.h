#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#define BUFFER_SIZE 64
#include "Sensor.h"
#include "../Actions/SensorAction.h"
#include <memory>

class LightSensor : public Sensor {
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
  LightSensor(int sensorPin, int detectionThreshold, SensorLocation loc, std::unique_ptr<SensorAction> sensorAction = nullptr);
      
  int readLevel();
  bool isTrainPassingOver(int lightReading);
  
  // Override methods from Sensor base class
  bool detectPassingTrain() override;
  bool isTrainDetected() const override;
  void reset() override;
  void executeAction(TrainController& controller, ActionController& actionController) override;
  
  SensorLocation getLocation() const override { return location; }
};

#endif // LIGHT_SENSOR_H