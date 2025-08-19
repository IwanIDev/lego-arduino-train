#include "Train/TrainInstance.hpp"
#include <Arduino.h>

TrainInstance::TrainInstance(const TrainConfig& trainConfig, size_t id, PositionTracker* positionTracker)
    : config(trainConfig)
    , hubName(trainConfig.hubName)
    , instanceId(id)
    , connected(false)
    , lastConnectionAttempt(0)
{
    // Create the hub instance
    hub = std::unique_ptr<Lpf2Hub>(new Lpf2Hub());
    
    // Create controllers with the hub
    bluetoothController = std::unique_ptr<BluetoothController>(new BluetoothController(hub.get()));
    trainController = std::unique_ptr<TrainController>(new TrainController(trainConfig.motorPort));
    inputController = std::unique_ptr<InputController>(new InputController(trainController.get(), 
                                                      trainConfig.fastButtonPin, 
                                                      trainConfig.slowButtonPin));
    actionController = std::unique_ptr<ActionController>(new ActionController(trainController.get()));
    
    // Create position sensor controller if position tracker is provided
    if (positionTracker) {
        positionSensorController = std::unique_ptr<PositionSensorController>(new PositionSensorController(*positionTracker));
        actionController->setPositionController(positionSensorController.get());
    }
    
    Serial.print("Created TrainInstance: ");
    Serial.print(hubName);
    Serial.print(" (ID: ");
    Serial.print(instanceId);
    Serial.println(")");
}

TrainInstance::~TrainInstance() {
    stop();
}

bool TrainInstance::initialize() {
    Serial.print("Initializing train: ");
    Serial.println(hubName);
    
    // Set hub name if specified
    if (hubName.length() > 0) {
        bluetoothController->setHubName(hubName.c_str());
    }
    
    return true; // Basic initialization always succeeds
}

bool TrainInstance::connect() {
    lastConnectionAttempt = millis();
    
    if (bluetoothController->isConnected()) {
        connected = true;
        return true;
    }
    
    Serial.print("Attempting to connect train: ");
    Serial.println(hubName);
    
    connected = bluetoothController->connect();
    
    if (connected) {
        Serial.print("Successfully connected train: ");
        Serial.println(hubName);
    }
    
    return connected;
}

void TrainInstance::update() {
    if (!isConnected()) {
        return;
    }
    
    // Update action controller (handles delayed and sequential actions)
    actionController->update();
    
    // Update train controller state
    if (trainController->hasStateChanged()) {
        int speed = trainController->getSpeed(trainController->getState());
        bluetoothController->setMotorSpeed(config.motorPort, speed);
        trainController->clearStateChanged();
    }
}

void TrainInstance::stop() {
    if (trainController) {
        trainController->setState(SPEED::STOPPED);
    }
    
    if (isConnected()) {
        bluetoothController->setMotorSpeed(config.motorPort, 0);
    }
    
    if (actionController) {
        actionController->clearAllActions();
    }
}

void TrainInstance::emergencyStop() {
    stop(); // Same as regular stop for now, but could be enhanced
}

void TrainInstance::handleSerialInput() {
    if (inputController) {
        inputController->handleSerialInput();
    }
}

void TrainInstance::handleButtonInput() {
    if (inputController) {
        SPEED oldState = trainController->getState();
        inputController->handleButtonInput(oldState);
    }
}

void TrainInstance::printStatus() const {
    Serial.print("Train ");
    Serial.print(instanceId);
    Serial.print(" (");
    Serial.print(hubName);
    Serial.print("): ");
    Serial.print(isConnected() ? "CONNECTED" : "DISCONNECTED");
    
    if (trainController) {
        Serial.print(", Speed: ");
        Serial.print(trainController->getState());
        Serial.print(", Reverse: ");
        Serial.print(trainController->getReverse() ? "ON" : "OFF");
    }
    
    if (actionController) {
        Serial.print(", Actions: ");
        Serial.print(actionController->getActiveDelayedActionsCount());
        Serial.print("/");
        Serial.print(actionController->getActiveSequentialActionsCount());
    }
    
    Serial.println();
}

String TrainInstance::getStatusString() const {
    String status = "Train " + String(instanceId) + " (" + hubName + "): ";
    status += isConnected() ? "CONNECTED" : "DISCONNECTED";
    
    if (trainController) {
        status += ", Speed: " + String(trainController->getState());
        status += ", Reverse: " + String(trainController->getReverse() ? "ON" : "OFF");
    }
    
    return status;
}