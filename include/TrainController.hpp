#include "Lpf2Hub.h"

enum SPEED {
  STOPPED = 0,
  FAST = 30,
  SLOW = 15
};

class TrainController {
private:
  SPEED trainState;
  bool stateChanged;
  unsigned long previousMillis;
  const long speedSwitchInterval;
  byte port;

public:
  TrainController(byte motorPort);
  
  void setState(SPEED newState);
  SPEED getState();
  bool hasStateChanged();
  void clearStateChanged();
  
  int getSpeed(SPEED state);
  bool canUpdateSpeed();
  void updateSpeedTimer();
  
  void printState();
};
