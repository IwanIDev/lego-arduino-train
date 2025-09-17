#include "BluetoothController.h"
#include "Lpf2Hub.h"
#include <Arduino.h>

// Constructor for BluetoothController
BluetoothController::BluetoothController(Lpf2Hub* hub) 
: trainHub(hub), initialized(false), firstConnectAttempt(true)
{

}

bool BluetoothController::connect() {
    if (this->isFirstConnectAttempt()) {
        Serial.println("First connection attempt");
        trainHub->init();
        this->setFirstConnectAttempt(false);
    }

    if (!trainHub->isConnecting()) {
        Serial.println("Not currently connecting");
        trainHub->init();
        return false;
    }

    Serial.println("Attempting to connect to hub...");
    trainHub->connectHub();

    if (!this->isConnected()) {
        Serial.println("Failed to connect to hub");
        return false;
    }

    Serial.println("Connected to hub");
    return true;
}

bool BluetoothController::isConnected() const {
    // Check if trainHub exists
    if (!trainHub) {
        return false;
    }
    
    // Get the connection state safely with additional validation
    int hubConnected = (int) trainHub->isConnected();
    
    // Validate the return value makes sense (should be 0 or 1)
    // Any result other than 1 is treated as false.

    return (hubConnected == 1) ? true : false;
}

void BluetoothController::setMotorSpeed(byte port, int speed) {
        if (!isConnected() || !trainHub) {
                Serial.println("Cannot set motor speed: No connection or null hub");
                return;
        }
        
        // Additional safety checks - ensure hub is still properly connected
        if (!trainHub->isConnected()) {
                Serial.println("Warning: Hub connection lost during motor speed setting");
                return;
        }
        
        // Add extra validation for hub state
        if (!trainHub) {
                Serial.println("Error: trainHub is null");
                return;
        }
        
        // Add a small delay to ensure the Bluetooth stack is ready
        delay(10); // Increased delay for better stability
        
        // Final check before the potentially dangerous operation
        if (!trainHub->isConnected()) {
                Serial.println("Warning: Hub connection lost just before motor command");
                return;
        }
        
        // Attempt to set motor speed with basic error handling
        Serial.print("Setting motor speed: port=");
        Serial.print(port);
        Serial.print(", speed=");
        Serial.println(speed);
        
        trainHub->setBasicMotorSpeed(port, speed);
        
        Serial.println("Motor speed command sent successfully");
}

void BluetoothController::setHubName(const char* name) {
        if (!isConnected() || !trainHub) {
                return;
        }
        
        // Additional safety check - ensure hub is still properly connected
        if (!trainHub->isConnected()) {
                Serial.println("Warning: Hub connection lost during hub name setting");
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

bool BluetoothController::isFirstConnectAttempt() {
    return firstConnectAttempt;
}

bool BluetoothController::setFirstConnectAttempt(bool attempt) {
    firstConnectAttempt = attempt;
    return firstConnectAttempt;
}
