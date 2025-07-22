#include "BluetoothController.hpp"
#include "Lpf2Hub.h"
#include <Arduino.h>

// Constructor for BluetoothController
BluetoothController::BluetoothController(Lpf2Hub* hub) 
: trainHub(hub), initialized(false) 
{

}

bool BluetoothController::connect() {
    if (!trainHub->isConnected() && !trainHub->isConnecting()) {
        Serial.println("Initialising the train hub...");
        trainHub->init(); // Initialize the Lpf2Hub instance
    }

    // Connect flow: Search for BLE services and try to connect if the UUID of the hub is found
    if (trainHub->isConnecting()) {
        Serial.println("Connecting to train hub...");
        trainHub->connectHub();
        if (trainHub->isConnected()) {
            Serial.println("Connected to HUB");
            Serial.print("Hub address: ");
            Serial.println(trainHub->getHubAddress().toString().c_str());
            Serial.print("Hub name: ");
            Serial.println(trainHub->getHubName().c_str());
            return true;
        } else {
            Serial.println("Failed to connect to HUB");
            return false;
        }
    }
    return false;
}

bool BluetoothController::isConnected() const {
    return trainHub->isConnected();
}

void BluetoothController::setMotorSpeed(byte port, int speed) {
    if (!isConnected()) {
        return;
    }
    trainHub->setBasicMotorSpeed(port, speed);
}

void BluetoothController::setHubName(const char* name) {
    if (!isConnected()) {
        return;
    }
    trainHub->setHubName((char*) name);
}

String BluetoothController::getHubAddress() {
    if (!isConnected()) {
        Serial.println("Cannot get hub address: Not connected to hub");
        return String();
    }
    return trainHub->getHubAddress().toString().c_str();
}

String BluetoothController::getHubName() {
    if (!isConnected()) {
        Serial.println("Cannot get hub name: Not connected to hub");
        return String();
    }
    return trainHub->getHubName().c_str();
}