/**
 * Basic train motor controls
 * Iwan I
 * 2025-07-03
 */
#include <Ultrasonic.h>
#include "Lpf2Hub.h"

// create a hub instance
Lpf2Hub trainHub;
byte port = (byte)PoweredUpHubPort::B;

Ultrasonic ultrasonic(7); // DIN 7

enum SPEED {
  FAST,
  SLOW,
  STOPPED
};

const int SLOW_SPEED = 15;
const int FAST_SPEED = 30;

const int fastButton = 2;
const int slowButton = 4;

SPEED trainState = STOPPED;

unsigned long previousMillis = 0;
const long speedSwitchInterval = 250;

void setup() {
    Serial.begin(115200);

    pinMode(fastButton, INPUT_PULLUP);
    pinMode(slowButton, INPUT_PULLUP);
}

/*
* Calculates the current state based on button press.
*/
void setState() {
  int fastButtonState = digitalRead(fastButton);
  int slowButtonState = digitalRead(slowButton);

  if (fastButtonState == LOW && slowButtonState == LOW) {
    trainState = STOPPED;
    Serial.println("STOPPED selected");
    delay(100);
  } else if (fastButtonState == LOW) {  // button pressed
    trainState = FAST;
    Serial.println("FAST selected");
  } else if (slowButtonState == LOW) {  // button pressed
    trainState = SLOW;
    Serial.println("SLOW selected");
  }
}

/*
* Determines the absolute speed values for the motors given
* the state.
*/
int getSpeed(SPEED state) {
  switch(state) {
    case SLOW: return SLOW_SPEED;
    case FAST: return FAST_SPEED;
    case STOPPED: return 0;
    default: return 0;
  }
}

/*
* Establish a connection to the train via BLE.
* Returns true if successful, false otherwise.
*/
bool connect() {
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
      return false;
    }
  }
  return true;
}

void handleInput() {
  const char STOP_COMMAND[] = "stop";
  const int STOP_COMMAND_LENGTH = sizeof(STOP_COMMAND) - 1;
  const char SLOW_COMMAND[] = "slow";
  const int SLOW_COMMAND_LENGTH = sizeof(STOP_COMMAND) - 1;
  const char FAST_COMMAND[] = "fast";
  const int FAST_COMMAND_LENGTH = sizeof(STOP_COMMAND) - 1;

  String recievedData = "";
  
  if (Serial.available() <= 0) {
    return;
  }

  recievedData = Serial.readStringUntil('\n');
  recievedData.trim();

  if (recievedData.equals(STOP_COMMAND)) trainState = STOPPED;
  if (recievedData.equals(SLOW_COMMAND)) trainState = SLOW;
  if (recievedData.equals(FAST_COMMAND)) trainState = FAST;
}

/*
* Check current train distance to sensor
*/
long checkDistance() {
  return ultrasonic.MeasureInCentimeters();
}

void applyStopAtDistance(long distance, long threshold) {
  if (distance >= threshold) return;
  if (distance <= 2) return;

  trainState = STOPPED;
}

void applySlowAtDistance(long distance, long threshold) {
  if (distance >= threshold) return;
  if (distance <= 2) return;

  if (trainState == FAST) trainState = SLOW; 
}

void loop() {
  unsigned long currentMillis = millis();
  unsigned long deltaT = currentMillis - previousMillis; // Time elapsed between last speed change and now.
  setState();
  handleInput();

  if (deltaT < speedSwitchInterval) { // If we haven't reached the interval to change speed we should not change the speed.
    return;
  }

  int value = analogRead(A0);
  Serial.printf("Light value: %d\n", value);


  long currentDistance = checkDistance();
  applySlowAtDistance(currentDistance, 30);
  applyStopAtDistance(currentDistance, 10);

  previousMillis = currentMillis;

  if (!connect()) {
    // Serial.println("Train hub is disconnected");
    return;
  }
    
  if (!trainHub.isConnected()) {
    // Serial.println("Train hub is disconnected");
    return;
  }

  Serial.write("Train state: ");
  Serial.println((int)trainState, DEC);
  
  char hubName[] = "trainHub";
  trainHub.setHubName(hubName);

  int speed = getSpeed(trainState);

  Serial.write("Speed: ");
  Serial.println(speed, DEC);

  Serial.write("Current distance: ");
  Serial.println(currentDistance, DEC);

  trainHub.setBasicMotorSpeed(port, speed);
}
