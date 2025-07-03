/*
* Lego Train Motor Test
* Iwan I
* 2025-07-03
*/

#include <Lpf2Hub.h>

Lpf2Hub train;
byte motorPort = (byte) PoweredUpHubPort::B;
char[] trainName = "testTrain";

int BAUD = 115200;

void setup() {
  Serial.begin(BAUD);
  train.init();
}

bool connectTrain() {
  train.connectHub();

  if (!train.isConnected()) {
    Serial.printf("Failed to connect to train %s", trainName);
    return false;
  }
  
  char[] address = train.getHubAddress().toString().c_str();
  Serial.printf("Connected to train %s on address %s", trainName, address);
  
  return true;
}

void loop() {
  if (train.isConnecting()) {
    connectTrain();
  }

  if (!train.isConnected()) {
    Serial.printf("Failed to connect to train %s", trainName);
    return;
  }
  
  train.setBasicMotorSpeed(port, 255);
}
