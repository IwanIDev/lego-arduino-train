/**
 * Basic train motor controls
 * Iwan I
 * 2025-07-15
 */
#include "Lpf2Hub.h"
#include "TrainController.hpp"
#include "LightSensor.hpp"

Lpf2Hub trainHub;
const byte MOTOR_PORT = (byte)PoweredUpHubPort::B;

TrainController trainController(MOTOR_PORT);

// ---------------------------
// --- Train Speed Control ---
// ---------------------------

const int fastButton = 2;
const int slowButton = 4;

unsigned long previousMillis = 0;
const long speedSwitchInterval = 100;

// ------------------------------------
// --- Light Sensor Train Detection ---
// ------------------------------------
const int LIGHT_SENSOR_PIN = A0; // Analogue pin 0
const int LIGHT_SENSOR_THRESHOLD = 500;
const int LIGHT_SENSOR_TIMEOUT_THRESHOLD = 500;

LightSensor lightSensor(LIGHT_SENSOR_PIN, LIGHT_SENSOR_THRESHOLD);

void setup() {
    Serial.begin(115200);

    pinMode(fastButton, INPUT_PULLUP);
    pinMode(slowButton, INPUT_PULLUP);
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
  
  if (Serial.available() <= 0) return;

  recievedData = Serial.readStringUntil('\n');
  recievedData.trim();

  if (recievedData.equals(STOP_COMMAND)) trainController.setState(STOPPED);
  if (recievedData.equals(SLOW_COMMAND)) trainController.setState(SLOW);
  if (recievedData.equals(FAST_COMMAND)) trainController.setState(FAST);
}


void loop() {
  unsigned long currentMillis = millis();
  unsigned long deltaT = currentMillis - previousMillis; // Time elapsed between last speed change and now.

  // ---------------------------
  // --- Set the train state ---
  // ---------------------------
  trainController.updateSpeedTimer();
  handleInput();

  if (lightSensor.detectPassingTrain()) {
    trainController.setState(STOPPED);
  }

  // ------------------------------------
  // --- Connect to train hub via BLE ---
  // ------------------------------------
  if (!connect()) {
    // Serial.println("Train hub is disconnected");
    return;
  }
    
  if (!trainHub.isConnected()) {
    // Serial.println("Train hub is disconnected");
    return;
  }

  if (deltaT < speedSwitchInterval) { // If we haven't reached the interval to change speed we should not change the speed.
    return;
  }

  previousMillis = currentMillis;

  // ---------------------------------------------
  // --- Output system state to serial console ---
  // ---------------------------------------------
  trainController.printState();
  
  char hubName[] = "trainHub";
  trainHub.setHubName(hubName);

  int speed = (int) trainController.getState();

  Serial.write(" Speed: ");
  Serial.print(speed, DEC);

  Serial.println();

  // --------------------------------
  // --- Set speed based on state ---
  // --------------------------------
  trainHub.setBasicMotorSpeed(MOTOR_PORT, speed);
}
