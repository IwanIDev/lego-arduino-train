#ifndef BLUETOOTH_CONTROLLER_H
#define BLUETOOTH_CONTROLLER_H

#include "Lpf2Hub.h"

class BluetoothController {
private:
  Lpf2Hub* trainHub;
  bool initialized;
  bool firstConnectAttempt;

  bool isFirstConnectAttempt();
  bool setFirstConnectAttempt(bool attempt);

public:
  BluetoothController(Lpf2Hub* hub);
  
  bool connect();
  bool isConnected() const;
  void setMotorSpeed(byte port, int speed);
  void setHubName(const char* name);
  
  String getHubAddress();
  String getHubName();
};

#endif // BLUETOOTH_CONTROLLER_H