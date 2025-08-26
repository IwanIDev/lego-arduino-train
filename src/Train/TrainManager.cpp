#include "Train/TrainManager.hpp"
#include "Train/TrainInstance.hpp"
#include <Arduino.h>

// TrainManager Implementation

TrainManager::TrainManager(PositionAwareSensorController* sensorController,
                          ReedSwitchSensorController* reedController,
                          LightSensorController* lightController)
    : positionAwareSensorController(sensorController)
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
    
    // Create new train instance (it will create its own position tracker)
    auto trainInstance = std::unique_ptr<TrainInstance>(new TrainInstance(config, trainIndex));
    
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

std::vector<SensorLocation> TrainManager::getAllTrainPositions() const {
    std::vector<SensorLocation> positions;
    for (const auto& train : trains) {
        if (train->getPositionTracker()) {
            positions.push_back(train->getPositionTracker()->getCurrentPosition());
        } else {
            positions.push_back(SensorLocation::UNKNOWN);
        }
    }
    return positions;
}

std::vector<TrainDirection> TrainManager::getAllTrainDirections() const {
    std::vector<TrainDirection> directions;
    for (const auto& train : trains) {
        if (train->getPositionTracker()) {
            directions.push_back(train->getPositionTracker()->getDirection());
        } else {
            directions.push_back(TrainDirection::FORWARD);
        }
    }
    return directions;
}

PositionTracker* TrainManager::getTrainPositionTracker(size_t trainIndex) {
    if (isValidTrainIndex(trainIndex)) {
        return trains[trainIndex]->getPositionTracker();
    }
    return nullptr;
}

PositionTracker* TrainManager::getTrainPositionTracker(const String& hubName) {
    auto it = hubNameToTrainIndex.find(hubName);
    if (it != hubNameToTrainIndex.end()) {
        return getTrainPositionTracker(it->second);
    }
    return nullptr;
}

// Private helper methods

void TrainManager::handlePositionUpdate() {
    if (!positionAwareSensorController) {
        return;
    }
    
    // Check for sensor triggers
    bool sensorTriggered = positionAwareSensorController->checkSensors();
    
    if (sensorTriggered) {
        SensorLocation triggeredLocation = positionAwareSensorController->getTriggeredLocation();
        
        if (triggeredLocation != SensorLocation::UNKNOWN) {
            // Find the train that should respond to this sensor trigger
            selectBestTrainForPosition(triggeredLocation);
        }
    }
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
    const int MAX_CONCURRENT_CONNECTIONS = 2;
    
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
            Serial.print("Connection result: ");
            Serial.println(connectionResult ? "Success" : "Failure");
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
    
    // For single train systems, update more frequently; for multi-train, space them out
    unsigned long updateInterval = (trains.size() <= 1) ? 10 : 50; // 10ms for single train, 50ms for multi-train
    
    // Space out train updates to avoid Bluetooth conflicts
    if (currentTime - lastTrainUpdateTime >= updateInterval) {
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

void TrainManager::executePositionBasedActions(size_t trainIndex, SensorLocation position, TrainDirection direction) {
    if (!isValidTrainIndex(trainIndex)) {
        return;
    }
    
    auto& train = trains[trainIndex];
    if (!train->isConnected()) {
        return;
    }
    
    // Get the track segment actions from this train's position tracker
    auto* positionTracker = train->getPositionTracker();
    if (!positionTracker) {
        return;
    }
    
    auto actions = positionTracker->getActionsForPosition(position, direction);
    
    // Execute each action through the train's action controller
    auto* actionController = train->getActionController();
    auto* trainController = train->getTrainController();
    
    if (actionController && trainController && !actions.empty()) {                
        for (auto& action : actions) {
            if (action) {
                // Check if this is a SequentialAction and we already have active ones
                if (action->isSequentialAction() && actionController->hasActiveSequentialActions()) {
                    Serial.println("SequentialAction already active, skipping duplicate position-based execution");
                    continue;
                }
                
                // Handle SequentialActions specially - they need to be managed by ActionController
                if (action->isSequentialAction()) {
                    Serial.println("TrainManager: Adding SequentialAction to ActionController for managed execution");
                    // Cast to SequentialAction and create a fresh copy for the ActionController
                    SequentialAction* sequentialAction = static_cast<SequentialAction*>(action.get());
                    actionController->addSequentialAction(sequentialAction->createFresh());
                } else {
                    // For immediate actions, execute directly
                    action->execute(*trainController, *actionController);
                }
            }
        }
    }
}

void TrainManager::selectBestTrainForPosition(SensorLocation position) {
    // Select the train that is in a position adjacent to the specified position.
    // If none, ignore the request.

    for (size_t i = 0; i < trains.size(); i++) {
        auto& train = trains[i];
        if (!train->isConnected()) continue;

        auto* positionTracker = train->getPositionTracker();
        if (!positionTracker) continue;

        // Store previous position and direction to detect changes
        SensorLocation previousPosition = positionTracker->getCurrentPosition();
        TrainDirection previousDirection = positionTracker->getDirection();
        
        if (positionTracker->getNextExpectedPosition() != position) {
            // Not the right train for this position
            continue;
        }

        // Update this train's position
        positionTracker->updatePosition(position);
        
        // Check if position or direction changed
        SensorLocation currentPosition = positionTracker->getCurrentPosition();
        TrainDirection currentDirection = positionTracker->getDirection();
        bool positionChanged = (previousPosition != currentPosition);
        bool directionChanged = (previousDirection != currentDirection);
        
        // Execute position-based actions if position changed OR direction changed
        if (positionChanged || directionChanged) {
            executePositionBasedActions(i, currentPosition, currentDirection);
        }

        break; // Only one train should respond to a position trigger
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
