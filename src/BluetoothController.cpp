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
        trainHub->init(); // initalize the PoweredUpHub instance
    }

    if (!trainHub->isConnecting()) {
        // If we are already connected, we do not need to connect again.
        return true;
    }

    trainHub->connectHub();

    if (!trainHub->isConnected()) {
        Serial.println("Failed to connect to HUB");
        return false;
    }

    Serial.println("Connected to HUB");
    Serial.print("Hub address: ");
    Serial.println(this->getHubAddress().c_str());
    Serial.print("Hub name: ");
    Serial.println(this->getHubName().c_str());

    return true;
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