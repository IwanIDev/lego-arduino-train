class LightSensor {
private:
  int pin;
  int threshold;
  int lastReading;
  bool trainDetected;
  unsigned long timeout;
  const unsigned long timeoutThreshold;

public:
  LightSensor(int sensorPin, int detectionThreshold);
  
  int readLevel();
  bool isTrainPassingOver(int lightReading);
  bool detectPassingTrain();
  
  bool isTrainDetected() const;
  void reset();
};
