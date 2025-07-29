#include "Lpf2Hub.h"
#include <Arduino.h>
#include "TrainController.hpp"
#include "BluetoothController.hpp"
#include "InputController.hpp"
#include "LightSensor.hpp"
#include "LightSensorController.hpp"

Lpf2Hub trainHub;
const byte MOTOR_PORT = (byte)PoweredUpHubPort::B;

const int fastButton = D2;
const int slowButton = D4;

unsigned long previousMillis = 0;
const long speedSwitchInterval = 100;

const int LIGHT_SENSOR_PINS[] = {
    A0,
}; 
const int LIGHT_SENSOR_THRESHOLD = 20; // Percentage threshold for light level detection
const int LIGHT_SENSOR_TIMEOUT_THRESHOLD = 500;

BluetoothController bluetoothController(&trainHub);
TrainController trainController(MOTOR_PORT);
InputController inputController(&trainController, fastButton, slowButton);
LightSensor sensor = LightSensor(LIGHT_SENSOR_PINS[0], LIGHT_SENSOR_THRESHOLD);
LightSensorController lightSensorController;

void setup() {
    Serial.begin(115200);
    lightSensorController.addSensor(&sensor);
}

void loop() {
    unsigned long currentMillis = millis();
    unsigned long deltaT = currentMillis - previousMillis; // Time elapsed between last speed change and now.

    trainController.updateSpeedTimer();
    inputController.handleSerialInput();
    inputController.handleButtonInput(trainController.getState());

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
