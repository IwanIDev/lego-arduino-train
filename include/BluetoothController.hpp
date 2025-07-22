#include "Lpf2Hub.h"

class BluetoothController {
private:
  Lpf2Hub* trainHub;
  bool initialized;

public:
  BluetoothController(Lpf2Hub* hub);
  
  bool connect();
  bool isConnected() const;
  void setMotorSpeed(byte port, int speed);
  void setHubName(const char* name);
  
  String getHubAddress();
  String getHubName();
};
