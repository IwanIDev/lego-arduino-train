#include "Train/TrainManager.hpp"
#include "Train/TrainInstance.hpp"
#include <Arduino.h>

// TrainManager Implementation

TrainManager::TrainManager(PositionAwareSensorController* sensorController,
                          PositionTracker* tracker,
                          ReedSwitchSensorController* reedController,
                          LightSensorController* lightController)
    : positionAwareSensorController(sensorController)
    , positionTracker(tracker)
    , reedSwitchSensorController(reedController)
    , lightSensorController(lightController)
    , initialized(false)
    , lastUpdate(0)
    , lastBluetoothAttempt(0)
    , lastDebugPrint(0)
{
}

TrainManager::~TrainManager() {
    stop();
}

size_t TrainManager::addTrain(const TrainConfig& config) {
    size_t trainIndex = trains.size();
    
    // Create new train instance
    auto trainInstance = std::unique_ptr<TrainInstance>(new TrainInstance(config, trainIndex, positionTracker));
    
    // Store the mapping from hub name to train index
    hubNameToTrainIndex[config.hubName] = trainIndex;
    
    // Add to trains vector
    trains.push_back(std::move(trainInstance));
    
    Serial.print("Added train: ");
    Serial.print(config.hubName);
    Serial.print(" at index: ");
    Serial.println(trainIndex);
    
    return trainIndex;
}

bool TrainManager::removeTrain(const String& hubName) {
    auto it = hubNameToTrainIndex.find(hubName);
    if (it != hubNameToTrainIndex.end()) {
        return removeTrain(it->second);
    }
    return false;
}

bool TrainManager::removeTrain(size_t trainIndex) {
    if (!isValidTrainIndex(trainIndex)) {
        return false;
    }
    
    // Stop the train before removing
    trains[trainIndex]->stop();
    
    // Remove from hub name mapping
    String hubName = trains[trainIndex]->getHubName();
    hubNameToTrainIndex.erase(hubName);
    
    // Remove from trains vector
    trains.erase(trains.begin() + trainIndex);
    
    // Update indices in the mapping
    for (auto& pair : hubNameToTrainIndex) {
        if (pair.second > trainIndex) {
            pair.second--;
        }
    }
    
    Serial.print("Removed train: ");
    Serial.println(hubName);
    
    return true;
}

bool TrainManager::initialize() {
    if (initialized) {
        return true;
    }
    
    Serial.println("Initializing TrainManager...");
    
    // Initialize all train instances
    bool allInitialized = true;
    for (auto& train : trains) {
        if (!train->initialize()) {
            allInitialized = false;
            Serial.print("Failed to initialize train: ");
            Serial.println(train->getHubName());
        }
    }
    
    initialized = allInitialized;
    
    if (initialized) {
        Serial.println("TrainManager initialized successfully");
    } else {
        Serial.println("TrainManager initialization failed");
    }
    
    return initialized;
}

void TrainManager::update() {
    unsigned long currentTime = millis();
    
    // Throttle updates to avoid overwhelming the system
    if (currentTime - lastUpdate < UPDATE_INTERVAL) {
        return;
    }
    lastUpdate = currentTime;
    
    // Update position tracking
    handlePositionUpdate();
    
    // Update all train connections
    updateTrainConnections();
    
    // Update all train controllers
    updateTrainControllers();
    
    // Debug printing
    if (currentTime - lastDebugPrint > DEBUG_PRINT_INTERVAL) {
        debugPrintActiveActions();
        lastDebugPrint = currentTime;
    }
}

void TrainManager::stop() {
    Serial.println("Stopping all trains...");
    for (auto& train : trains) {
        train->stop();
    }
}

void TrainManager::emergencyStop() {
    Serial.println("EMERGENCY STOP - All trains stopping immediately!");
    for (auto& train : trains) {
        train->emergencyStop();
    }
}

TrainInstance* TrainManager::getTrain(const String& hubName) {
    auto it = hubNameToTrainIndex.find(hubName);
    if (it != hubNameToTrainIndex.end()) {
        return getTrain(it->second);
    }
    return nullptr;
}

TrainInstance* TrainManager::getTrain(size_t trainIndex) {
    if (isValidTrainIndex(trainIndex)) {
        return trains[trainIndex].get();
    }
    return nullptr;
}

// Private helper methods

void TrainManager::handlePositionUpdate() {
    if (!positionAwareSensorController) {
        return;
    }
    
    // Update sensor readings
    positionAwareSensorController->checkSensorsAndUpdatePosition();
    
    // Check for position changes and execute position-based actions
    SensorLocation currentPosition = positionTracker->getCurrentPosition();
    TrainDirection direction = positionTracker->getDirection();
    
    // Execute position-based actions for all trains
    executePositionBasedActions(currentPosition, direction);
}

void TrainManager::updateTrainConnections() {
    unsigned long currentTime = millis();
    
    // Only attempt connections at specified intervals
    if (currentTime - lastBluetoothAttempt < BLUETOOTH_RETRY_INTERVAL) {
        return;
    }
    
    // Count how many trains are already connected
    int connectedTrains = 0;
    for (auto& train : trains) {
        if (train->isConnected()) {
            connectedTrains++;
        }
    }
    
    // If we have a single hub setup, only allow one train to connect
    // TODO: Make this configurable based on actual hardware setup
    const int MAX_CONCURRENT_CONNECTIONS = 1;
    
    if (connectedTrains >= MAX_CONCURRENT_CONNECTIONS) {
        // Skip connection attempts if we're at the limit
        lastBluetoothAttempt = currentTime;
        return;
    }
    
    // Try to connect one disconnected train
    for (auto& train : trains) {
        if (!train->isConnected()) {
            Serial.print("Attempting connection for train: ");
            Serial.println(train->getHubName());
            
            bool connectionResult = train->connect();
            if (connectionResult) {
                connectedTrains++;
                if (connectedTrains >= MAX_CONCURRENT_CONNECTIONS) {
                    break; // Stop trying to connect more trains
                }
            }
            break; // Only try one train per update cycle
        }
    }
    
    lastBluetoothAttempt = currentTime;
}

void TrainManager::updateTrainControllers() {
    static unsigned long lastTrainUpdateTime = 0;
    static size_t currentTrainIndex = 0;
    unsigned long currentTime = millis();
    
    // Space out train updates to avoid Bluetooth conflicts
    if (currentTime - lastTrainUpdateTime >= 50) { // 50ms between train updates
        if (currentTrainIndex < trains.size()) {
            auto& train = trains[currentTrainIndex];
            
            // Only update trains that are actually connected
            if (train->isConnected()) {                
                train->update();
                train->handleSerialInput();
                train->handleButtonInput();
            }
            
            currentTrainIndex++;
            lastTrainUpdateTime = currentTime;
        } else {
            currentTrainIndex = 0; // Reset to first train
        }
    }
}

void TrainManager::executePositionBasedActions(SensorLocation position, TrainDirection direction) {
    // For multi-train systems, we need to determine which train should respond to position changes
    // For now, we'll apply actions to all connected trains, but this could be made more sophisticated
    selectBestTrainForPosition(position);
}

void TrainManager::selectBestTrainForPosition(SensorLocation position) {
    // Simple strategy: apply position-based actions to the first connected train
    // In a more sophisticated system, this could consider proximity, train schedules, etc.
    for (auto& train : trains) {
        if (train->isConnected()) {
            auto* positionController = train->getPositionSensorController();
            if (positionController) {
                // The position controller will handle the actual action execution
                // based on the current position and direction
                break; // Only apply to first connected train for now
            }
        }
    }
}

bool TrainManager::isValidTrainIndex(size_t index) const {
    return index < trains.size();
}

void TrainManager::debugPrintActiveActions() {
    bool hasActiveTrains = false;
    
    for (size_t i = 0; i < trains.size(); i++) {
        auto& train = trains[i];
        if (train->isConnected()) {
            auto* actionController = train->getActionController();
            if (actionController && actionController->hasActiveActions()) {
                if (!hasActiveTrains) {
                    Serial.println("=== Active Actions Debug ===");
                    hasActiveTrains = true;
                }
                
                Serial.print("Train ");
                Serial.print(i);
                Serial.print(" (");
                Serial.print(train->getHubName());
                Serial.print("): ");
                Serial.print(actionController->getActiveDelayedActionsCount());
                Serial.print(" delayed, ");
                Serial.print(actionController->getActiveSequentialActionsCount());
                Serial.println(" sequential");
            }
        }
    }
    
    if (hasActiveTrains) {
        Serial.println("========================");
    }
}
