#include "Lpf2Hub.h"
#include <Arduino.h>
#include "TrainController.hpp"
#include "BluetoothController.hpp"
#include "InputController.hpp"
#include "LightSensor.hpp"
#include "LightSensorController.hpp"
#include "Action/SensorAction.hpp"
#include "Action/StopAction.hpp"
#include "Action/ReverseAction.hpp"
#include "ActionController.hpp"
#include "ReedSwitchSensor.hpp"
#include "ReedSwitchSensorController.hpp"
#include <memory>

Lpf2Hub trainHub;
const byte MOTOR_PORT = (byte)PoweredUpHubPort::B;

const int fastButton = D2;
const int slowButton = D4;

unsigned long previousMillis = 0;
const long speedSwitchInterval = 100;

const int LIGHT_SENSOR_THRESHOLD = 20; // Percentage threshold for light level detection
const int LIGHT_SENSOR_TIMEOUT_THRESHOLD = 500;

BluetoothController bluetoothController(&trainHub);
TrainController trainController(MOTOR_PORT);
InputController inputController(&trainController, fastButton, slowButton);
LightSensor sensors[] = {
    LightSensor(A0, LIGHT_SENSOR_THRESHOLD, SensorLocation::STATION_STOP, std::unique_ptr<StopAction>(new StopAction(100))),
    // LightSensor(A1, LIGHT_SENSOR_THRESHOLD, SensorLocation::DIRECTION_CHANGE, std::unique_ptr<ReverseAction>(new ReverseAction(0))),
    // LightSensor(A2, LIGHT_SENSOR_THRESHOLD, SensorLocation::SPEED_REDUCE)
};
LightSensorController lightSensorController;
ReedSwitchSensor reedSwitchSensors[] = {
    ReedSwitchSensor(D12, SensorLocation::STATION_STOP, std::unique_ptr<StopAction>(new StopAction(100))),
    ReedSwitchSensor(D10, SensorLocation::STATION_STOP, std::unique_ptr<ReverseAction>(new ReverseAction(100))),
    // Add more ReedSwitchSensors as needed
};
ReedSwitchSensorController reedSwitchSensorController;
ActionController actionController(&trainController);

void setup() {
    Serial.begin(115200);
    for (auto& sensor : sensors) {
        lightSensorController.addSensor(&sensor);
    }
    for (auto& sensor : reedSwitchSensors) {
        reedSwitchSensorController.addSensor(&sensor);
    }
}

void loop() {
    unsigned long currentMillis = millis();
    unsigned long deltaT = currentMillis - previousMillis; // Time elapsed between last speed change and now.

    trainController.updateSpeedTimer();
    inputController.handleSerialInput();
    inputController.handleButtonInput(trainController.getState());

    if (lightSensorController.isTrainPassingOver()) {
        LightSensor* triggeredSensor = lightSensorController.getTriggeredSensor();
        actionController.executeAction(triggeredSensor);
    }

    if (reedSwitchSensorController.isTrainPassingOver()) {
        ReedSwitchSensor* triggeredSensor = reedSwitchSensorController.getTriggeredSensor();
        actionController.executeAction(triggeredSensor);
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

    SPEED currentState = trainController.getState();
    int speed = trainController.getSpeed(currentState);

    trainController.printState();

    bluetoothController.setMotorSpeed(MOTOR_PORT, speed);
}
