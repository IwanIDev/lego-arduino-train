#ifndef TRAINCONTROLLER_HPP
#define TRAINCONTROLLER_HPP

#include "Lpf2Hub.h"

#ifndef SPEED_ENUM_DEFINED
#define SPEED_ENUM_DEFINED
enum SPEED {
  STOPPED = 0,
  GO = 15
};
#endif // SPEED_ENUM_DEFINED

class TrainController {
private:
  SPEED trainState;
  bool stateChanged;
  unsigned long previousMillis;
  const long speedSwitchInterval;
  byte port;
  bool isReverse = false; // Added to track reverse state

  float speedMultiplier; // Speed multiplier
  const float MIN_MULTIPLIER = 0.0f; // Minimum speed multiplier
  const float MAX_MULTIPLIER = 1.0f; // Maximum speed multiplier
  const float MULTIPLIER_STEP = 0.2f; // Step size for speed multiplier adjustment (gives 5 speed levels)

public:
  bool setReverse(bool reverse) {
    isReverse = reverse;
    return isReverse;
  }
  bool getReverse() const {
    return isReverse;
  }
  TrainController(byte motorPort);
  
  void setState(SPEED newState);
  SPEED getState();
  bool hasStateChanged();
  void clearStateChanged();
  
  int getSpeed(SPEED state);
  bool canUpdateSpeed();
  void updateSpeedTimer();
  
  void printState();

  void incrementSpeed();
  void decrementSpeed();
  float getSpeedMultiplier() const {
    return speedMultiplier;
  }
};

#endif // TRAINCONTROLLER_HPP