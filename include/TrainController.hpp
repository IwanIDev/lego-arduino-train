#ifndef TRAINCONTROLLER_HPP
#define TRAINCONTROLLER_HPP

#include "Lpf2Hub.h"
#include <map>

#ifndef SPEED_ENUM_DEFINED
#define SPEED_ENUM_DEFINED
enum SPEED {
  STOPPED = 0,
  GO = 15
};
#endif // SPEED_ENUM_DEFINED

// Forward declaration
class TrainController;

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

  Lpf2Hub* hub; // Reference to the train hub
  uint8_t batteryVoltage; // Battery voltage level (0-100%)
  unsigned long lastBatteryUpdate; // Timestamp of last battery voltage update
  const unsigned long BATTERY_UPDATE_INTERVAL = 30000; // Update battery every 30 seconds

public:
  bool setReverse(bool reverse) {
    isReverse = reverse;
    return isReverse;
  }
  bool getReverse() const {
    return isReverse;
  }
  TrainController(byte motorPort, Lpf2Hub* trainHub);
  ~TrainController(); // Destructor to clean up the hub mapping
  
  void setState(SPEED newState);
  SPEED getState();
  bool hasStateChanged();
  void clearStateChanged();

  void setSpeedMultiplier(float multiplier);
  float getSpeedMultiplier() const;

  int getSpeed(SPEED state);
  bool canUpdateSpeed();
  void updateSpeedTimer();
  
  void printState();

  void incrementSpeed();
  void decrementSpeed();

  // Battery voltage methods
  uint8_t getBatteryVoltage() const;
  void updateBatteryVoltage();
  bool shouldUpdateBatteryVoltage() const;

  // Static callback for battery voltage updates
  static void batteryVoltageCallback(void* hub, HubPropertyReference property, uint8_t* data);
};

// Global map to associate hubs with their TrainController instances
extern std::map<Lpf2Hub*, TrainController*> hubToControllerMap;

#endif // TRAINCONTROLLER_HPP