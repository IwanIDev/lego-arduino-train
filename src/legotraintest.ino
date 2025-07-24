#include "Lpf2Hub.h"
#include <Arduino.h>
#include "TrainController.hpp"
#include "BluetoothController.hpp"
#include "InputController.hpp"
#include "LightSensor.hpp"
#include "LightSensorController.hpp"

Lpf2Hub trainHub;
const byte MOTOR_PORT = (byte)PoweredUpHubPort::B;

BluetoothController bluetoothController(&trainHub);
TrainController trainController(MOTOR_PORT);

// ---------------------------
// --- Train Speed Control ---
// ---------------------------

const int fastButton = D2;
const int slowButton = D4;

InputController inputController(&trainController, fastButton, slowButton);

unsigned long previousMillis = 0;
const long speedSwitchInterval = 100;

// ------------------------------------
// --- Light Sensor Train Detection ---
// ------------------------------------
const int LIGHT_SENSOR_PIN = A0; // Analogue pin 0
const int LIGHT_SENSOR_THRESHOLD = 20; // Percentage threshold for light level detection
const int LIGHT_SENSOR_TIMEOUT_THRESHOLD = 500;

LightSensor lightSensor(LIGHT_SENSOR_PIN, LIGHT_SENSOR_THRESHOLD);
LightSensorController lightSensorController;

void setup() {
    Serial.begin(115200);
    lightSensorController.addSensor(&lightSensor);
}

void loop() {
    unsigned long currentMillis = millis();
    unsigned long deltaT = currentMillis - previousMillis; // Time elapsed between last speed change and now.

    trainController.updateSpeedTimer();
    inputController.handleSerialInput();
    inputController.handleButtonInput();

    if (lightSensorController.isTrainPassingOver()) {
        trainController.setState(SPEED::STOPPED);
    }

    if (!bluetoothController.connect()) {
        return;
    }

    if (!bluetoothController.isConnected()) {
        return;
    }

    if (deltaT < speedSwitchInterval) { // If we haven't reached the interval to change speed we should not change the speed.
        return;
    }

    previousMillis = currentMillis;

    char hubName[] = "trainHub";
    trainHub.setHubName(hubName);

    int speed = (int) trainController.getState();

    trainController.printState();

    bluetoothController.setMotorSpeed(MOTOR_PORT, speed);
}
