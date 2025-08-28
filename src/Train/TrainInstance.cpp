#include "Train/TrainInstance.hpp"
#include <Arduino.h>

TrainInstance::TrainInstance(const TrainConfig& trainConfig, size_t id)
    : config(trainConfig)
    , hubName(trainConfig.hubName)
    , instanceId(id)
    , connected(false)
    , lastConnectionAttempt(0)
    , connectionTimestamp(0)
    , lastCommandTimestamp(0)
    , firstCommandSent(false)
{
    // Create the hub instance
    hub = std::unique_ptr<Lpf2Hub>(new Lpf2Hub());
    
    // Create the position tracker for this train instance
    positionTracker = std::unique_ptr<PositionTracker>(new PositionTracker(trainConfig.initialPosition));
    
    // Create controllers with the hub
    bluetoothController = std::unique_ptr<BluetoothController>(new BluetoothController(hub.get()));
    trainController = std::unique_ptr<TrainController>(new TrainController(trainConfig.motorPort, hub.get()));
    inputController = std::unique_ptr<InputController>(new InputController(trainController.get(), 
                                                      trainConfig.fastButtonPin, 
                                                      trainConfig.slowButtonPin));
    actionController = std::unique_ptr<ActionController>(new ActionController(trainController.get()));
    
    // Create position sensor controller with this train's position tracker
    positionSensorController = std::unique_ptr<PositionSensorController>(new PositionSensorController(*positionTracker));
    actionController->setPositionController(positionSensorController.get());
    
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
    
    // Basic initialization - don't set hub name until connected
    return true; // Basic initialization always succeeds
}

bool TrainInstance::connect() {
    unsigned long now = millis();
    if (now - lastConnectionAttempt < 500) {
        // Only attempt connection every 500 ms
        return this->isConnected();
    }

    Serial.print("Attempting to connect train: ");
    Serial.println(hubName);

    if (bluetoothController->isConnected() && lastConnectionAttempt != 0) {
        Serial.println("Already connected!");
        connected = true;
        return true;
    }

    lastConnectionAttempt = now;
        
    if (!bluetoothController->connect()) {
        Serial.print("Failed to connect train: ");
        Serial.println(hubName);
        return false;
    }
    
    // Wait a bit for the connection to stabilize
    delay(100);
    
    // Verify the connection is actually stable
    if (!bluetoothController->isConnected()) {
        // Connection was reported as successful but isn't stable
        Serial.print("Connection unstable for train: ");
        Serial.println(hubName);
        connected = false;
        firstCommandSent = false;
        return false;
    }

    Serial.print("Successfully connected train: ");
    Serial.println(hubName);
    connectionTimestamp = millis(); // Record when we connected
    
    // Set hub name if specified and we're connected
    if (hubName.length() > 0) {
        bluetoothController->setHubName(hubName.c_str());
    }

    return true;
}

void TrainInstance::update() {
    // Validate connection state before proceeding
    if (!validateConnection()) {
        return;
    }
    
    // Update action controller (handles delayed and sequential actions)
    actionController->update();
    
    // Update battery voltage if needed
    if (trainController->shouldUpdateBatteryVoltage()) {
        trainController->updateBatteryVoltage();
    }
    
    // Update train controller state
    if (trainController->hasStateChanged()) {
        Serial.print("State changed detected for train: ");
        Serial.println(hubName);
        
        // Triple-check connection before sending motor commands
        if (connected && bluetoothController && bluetoothController->isConnected()) {
            Serial.print("Train ");
            Serial.print(hubName);
            Serial.print(" connection checks passed");
            Serial.print(" - connected: ");
            Serial.print(connected);
            Serial.print(", bluetoothController exists: ");
            Serial.print(bluetoothController ? "yes" : "no");
            Serial.print(", isConnected: ");
            Serial.println(bluetoothController->isConnected());
            
            unsigned long currentTime = millis();
            bool canSendCommand = false;
            
            if (!firstCommandSent) {
                // For the first command after connection, wait at least 200ms
                if (currentTime - connectionTimestamp > 200) {
                    canSendCommand = true;
                    firstCommandSent = true;
                    lastCommandTimestamp = currentTime; // Initialize last command time
                    Serial.println("Sending first motor command after connection");
                } else {
                    Serial.println("Connection too recent, waiting before sending first motor command");
                }
            } else {
                // For subsequent commands, only wait 50ms to avoid rapid-fire commands
                if (currentTime - lastCommandTimestamp > 50) {
                    canSendCommand = true;
                    Serial.println("Sending motor command");
                }
            }
            
            if (canSendCommand) {
                // Add a small delay to ensure Bluetooth stack is ready
                delay(5);
                
                // Check one more time before sending the command
                if (bluetoothController->isConnected()) {
                    int speed = trainController->getSpeed(trainController->getState());
                    Serial.print("Train ");
                    Serial.print(hubName);
                    Serial.print(" sending motor command with speed: ");
                    Serial.println(speed);
                    bluetoothController->setMotorSpeed(config.motorPort, speed);
                    lastCommandTimestamp = currentTime; // Update last command time
                    trainController->clearStateChanged();
                } else {
                    Serial.print("Connection lost just before motor command for train: ");
                    Serial.println(hubName);
                    connected = false;
                    firstCommandSent = false;
                }
            }
        } else {
            // Connection lost during update, mark as disconnected
            connected = false;
            firstCommandSent = false;
            Serial.println("Connection lost during train update");
        }
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
    if (inputController && trainController) {
        SPEED oldState = trainController->getState();
        bool oldStateChanged = trainController->hasStateChanged();
        
        inputController->handleButtonInput(oldState);
        
        // Check if state changed due to button press
        if (trainController->hasStateChanged() && !oldStateChanged) {
            Serial.print("Button input caused state change for train: ");
            Serial.println(hubName);
        }
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
        Serial.print(", Battery: ");
        Serial.print(trainController->getBatteryVoltage());
        Serial.print("%");
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
        status += ", Battery: " + String(trainController->getBatteryVoltage()) + "%";
    }
    
    return status;
}

bool TrainInstance::validateConnection() {
    // If we think we're not connected, that's fine
    if (!connected) {
        return false;
    }
    
    // If we've been connected for less than 100ms, consider it unstable
    unsigned long currentTime = millis();
    if (currentTime - connectionTimestamp < 100) {
        return false; // Too recent, don't trust it yet
    }
    
    // If we think we're connected, verify with the Bluetooth controller
    if (bluetoothController && bluetoothController->isConnected()) {
        return true; // All good
    }
    
    // Connection mismatch detected - but give it a chance for transient issues
    static unsigned long lastValidationFailure = 0;
    
    if (currentTime - lastValidationFailure > 1000) { // Only log once per second
        Serial.print("Connection validation failed for train: ");
        Serial.println(hubName);
        lastValidationFailure = currentTime;
    }
    
    // Mark as disconnected so reconnection can be attempted
    connected = false;
    connectionTimestamp = 0; // Reset timestamp
    firstCommandSent = false; // Reset first command flag
    return false;
}