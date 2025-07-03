/**
 * Basic train motor controls
 * Iwan I
 * 2025-07-03
 */

#include "Lpf2Hub.h"

// create a hub instance
Lpf2Hub trainHub;
byte port = (byte)PoweredUpHubPort::B;

enum SPEED {
  FAST,
  SLOW,
  STOPPED
};

const int fastButton = 2;
const int slowButton = 4;

SPEED trainState = FAST;

void setup() {
    Serial.begin(115200);

    pinMode(fastButton, INPUT_PULLUP);
    pinMode(slowButton, INPUT_PULLUP);
}

void setState() {
  int fastButtonState = digitalRead(fastButton);
  int slowButtonState = digitalRead(slowButton);

  if (fastButtonState == LOW) {  // button pressed
    trainState = FAST;
    Serial.println("FAST selected");
  } else if (slowButtonState == LOW) {  // button pressed
    trainState = SLOW;
    Serial.println("SLOW selected");
  }

}

int getSpeed(SPEED state) {
  switch(state) {
    case SLOW:
      return 35;
    case FAST:
      return 50;
    case STOPPED:
      return 0;
  }
}

// main loop
void loop() {

  if (!trainHub.isConnected() && !trainHub.isConnecting()) {
    trainHub.init(); // initalize the PoweredUpHub instance
    //trainHub.init("90:84:2b:03:19:7f"); //example of initializing an hub with a specific address
  }

  // connect flow. Search for BLE services and try to connect if the uuid of the hub is found
  if (trainHub.isConnecting()) {
    trainHub.connectHub();
    if (trainHub.isConnected()) {
      Serial.println("Connected to HUB");
      Serial.print("Hub address: ");
      Serial.println(trainHub.getHubAddress().toString().c_str());
      Serial.print("Hub name: ");
      Serial.println(trainHub.getHubName().c_str());
    } else {
      Serial.println("Failed to connect to HUB");
    }
  }

  setState();

  // if connected, you can set the name of the hub, the led color and shut it down
  if (trainHub.isConnected()) {

    char hubName[] = "trainHub";
    trainHub.setHubName(hubName);


    int speed = getSpeed(trainState);

    trainHub.setBasicMotorSpeed(port, speed);
    
  } else {
    Serial.println("Train hub is disconnected");
    delay(1000);
  }

} // End of loop
