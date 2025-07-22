/**
 * Basic train motor controls
 * Iwan I
 * 2025-07-15
 */
#include "Lpf2Hub.h"
#include "TrainController.hpp"
#include "LightSensor.hpp"
#include "BluetoothController.hpp"

Lpf2Hub trainHub;
const byte MOTOR_PORT = (byte)PoweredUpHubPort::B;

BluetoothController bluetoothController(&trainHub);
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
  if (!bluetoothController.connect()) {
    // Serial.println("Train hub is disconnected");
    return;
  }

  if (!bluetoothController.isConnected()) {
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
  bluetoothController.setMotorSpeed(MOTOR_PORT, speed);
}
