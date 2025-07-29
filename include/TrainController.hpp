#ifndef TRAINCONTROLLER_HPP
#define TRAINCONTROLLER_HPP

#include "Lpf2Hub.h"

#ifndef SPEED_ENUM_DEFINED
#define SPEED_ENUM_DEFINED
enum SPEED {
  STOPPED = 0,
  FAST = 30,
  SLOW = 15
};
#endif // SPEED_ENUM_DEFINED

class TrainController {
private:
  SPEED trainState;
  bool stateChanged;
  unsigned long previousMillis;
  const long speedSwitchInterval;
  byte port;

public:
  bool isReverse = false; // Added to track reverse state

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

#endif // TRAINCONTROLLER_HPP