#ifndef TRAIN_MANAGER_H
#define TRAIN_MANAGER_H

#include "Lpf2Hub.h"
#include "../Controllers/TrainController.h"
#include "../Controllers/BluetoothController.h"
#include "../Controllers/InputController.h"
#include "../Controllers/ActionController.h"
#include "../Sensors/PositionAwareSensorController.h"
#include "../Position/PositionTracker.h"
#include "../Sensors/PositionSensorController.h"
#include "../Sensors/LightSensorController.h"
#include "../Sensors/ReedSwitchSensorController.h"
#include <vector>
#include <memory>
#include <map>

// Include TrainConfig definition
#include "TrainConfig.h"

// Forward declaration
class TrainInstance;

class TrainManager {
private:
    // Shared sensor infrastructure
    PositionAwareSensorController* positionAwareSensorController;
    ReedSwitchSensorController* reedSwitchSensorController;
    LightSensorController* lightSensorController;
    
    // Train instances
    std::vector<std::unique_ptr<TrainInstance>> trains;
    std::map<String, size_t> hubNameToTrainIndex;
    
    // System state
    bool initialized;
    unsigned long lastUpdate;
    static const unsigned long UPDATE_INTERVAL = 10; // 10ms update interval for more responsive sensor detection
    
    // Connection management
    unsigned long lastBluetoothAttempt;
    static const unsigned long BLUETOOTH_RETRY_INTERVAL = 5000; // 5 seconds
    
    // Debug and monitoring
    unsigned long lastDebugPrint;
    static const unsigned long DEBUG_PRINT_INTERVAL = 2000; // 2 seconds
    bool debugMode = false;
    
public:
    TrainManager(PositionAwareSensorController* sensorController,
                 ReedSwitchSensorController* reedController,
                 LightSensorController* lightController);
    
    ~TrainManager();
    
    // Train management
    size_t addTrain(const TrainConfig& config);
    bool removeTrain(const String& hubName);
    bool removeTrain(size_t trainIndex);
            
    // System control
    bool initialize();
    void update();
    void stop();
    void emergencyStop();
    
    // Train access
    TrainInstance* getTrain(const String& hubName);
    TrainInstance* getTrain(size_t trainIndex);
    size_t getTrainCount() const { return trains.size(); }
    
    // Position tracking for all trains
    std::vector<SensorLocation> getAllTrainPositions() const;
    std::vector<TrainDirection> getAllTrainDirections() const;
    PositionTracker* getTrainPositionTracker(size_t trainIndex);
    PositionTracker* getTrainPositionTracker(const String& hubName);
    
    // Debug control
    void setDebugMode(bool enable) { debugMode = enable; }
    bool getDebugMode() const { return debugMode; }
    
    // Status and debugging
    bool isInitialized() const { return initialized; }
    
private:
    // Internal helpers
    void handlePositionUpdate();
    void updateTrainConnections();
    void updateTrainControllers();
    void executePositionBasedActions(size_t trainIndex, SensorLocation position, TrainDirection direction);
    void selectBestTrainForPosition(SensorLocation position);
    bool isValidTrainIndex(size_t index) const;
    void debugPrintActiveActions();
};

#endif // TRAIN_MANAGER_H