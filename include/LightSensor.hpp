#define BUFFER_SIZE 64

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

  int getAverageLightLevel();

public:
  LightSensor(int sensorPin, int detectionThreshold);
  
  int readLevel();
  bool isTrainPassingOver(int lightReading);
  bool detectPassingTrain();
  
  bool isTrainDetected() const;
  void reset();
};
