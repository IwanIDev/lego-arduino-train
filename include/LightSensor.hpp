#ifndef LIGHTSENSOR_HPP
#define LIGHTSENSOR_HPP

#define BUFFER_SIZE 64
#include "Sensor.hpp"
#include "Action/SensorAction.hpp"
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
  LightSensor(int sensorPin, int detectionThreshold, SensorLocation loc, std::unique_ptr<SensorAction> sensorAction);
      
  int readLevel();
  bool isTrainPassingOver(int lightReading);
  
  // Override methods from Sensor base class
  bool detectPassingTrain() override;
  bool isTrainDetected() const override;
  void reset() override;
  void executeAction(TrainController& controller) override;
  
  SensorLocation getLocation() const { return location; }
};

#endif // LIGHTSENSOR_HPP
