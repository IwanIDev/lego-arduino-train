#ifndef TRAINMANAGER_HPP
#define TRAINMANAGER_HPP

#include "Lpf2Hub.h"
#include "TrainController.hpp"
#include "BluetoothController.hpp"
#include "InputController.hpp"
#include "ActionController.hpp"
#include "PositionAwareSensorController.hpp"
#include "PositionTracker.hpp"
#include "PositionSensorController.hpp"
#include "LightSensorController.hpp"
#include "ReedSwitchSensorController.hpp"
#include <vector>
#include <memory>
#include <map>

// Forward declaration
class TrainInstance;

struct TrainConfig {
    String hubName;
    byte motorPort;
    int fastButtonPin;
    int slowButtonPin;
};

class TrainManager {
private:
    // Shared sensor infrastructure
    PositionAwareSensorController* positionAwareSensorController;
    PositionTracker* positionTracker;
    ReedSwitchSensorController* reedSwitchSensorController;
    LightSensorController* lightSensorController;
    
    // Train instances
    std::vector<std::unique_ptr<TrainInstance>> trains;
    std::map<String, size_t> hubNameToTrainIndex;
    
    // System state
    bool initialized;
    unsigned long lastUpdate;
    static const unsigned long UPDATE_INTERVAL = 50; // 50ms update interval
    
    // Connection management
    unsigned long lastBluetoothAttempt;
    static const unsigned long BLUETOOTH_RETRY_INTERVAL = 5000; // 5 seconds
    
    // Debug and monitoring
    unsigned long lastDebugPrint;
    static const unsigned long DEBUG_PRINT_INTERVAL = 2000; // 2 seconds
    
public:
    TrainManager(PositionAwareSensorController* sensorController,
                 PositionTracker* tracker,
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
    
    // Status and debugging
    bool isInitialized() const { return initialized; }    
private:
    // Internal helpers
    void handlePositionUpdate();
    void updateTrainConnections();
    void updateTrainControllers();
    void executePositionBasedActions(SensorLocation position, TrainDirection direction);
    void selectBestTrainForPosition(SensorLocation position);
    bool isValidTrainIndex(size_t index) const;
    void debugPrintActiveActions();
};

// TrainInstance class - represents a single train with all its controllers
class TrainInstance {
private:
    // Core components
    std::unique_ptr<Lpf2Hub> hub;
    std::unique_ptr<TrainController> trainController;
    std::unique_ptr<BluetoothController> bluetoothController;
    std::unique_ptr<InputController> inputController;
    std::unique_ptr<ActionController> actionController;
    std::unique_ptr<PositionSensorController> positionSensorController;
    
    // Configuration
    TrainConfig config;
    String hubName;
    size_t instanceId;
    
    // State
    bool connected;
    unsigned long lastConnectionAttempt;
    
public:
    TrainInstance(const TrainConfig& trainConfig, size_t id, PositionTracker* positionTracker);
    ~TrainInstance();
    
    // Initialization
    bool initialize();
    
    // Connection management
    bool connect();
    bool isConnected() const { return connected && bluetoothController->isConnected(); }
    unsigned long getLastConnectionAttempt() const { return lastConnectionAttempt; }
    
    // Control
    void update();
    void stop();
    void emergencyStop();
    
    // Input handling
    void handleSerialInput();
    void handleButtonInput();
    
    // Configuration and state
    const String& getHubName() const { return hubName; }
    size_t getInstanceId() const { return instanceId; }
    
    // Component access
    TrainController* getTrainController() const { return trainController.get(); }
    BluetoothController* getBluetoothController() const { return bluetoothController.get(); }
    InputController* getInputController() const { return inputController.get(); }
    ActionController* getActionController() const { return actionController.get(); }
    PositionSensorController* getPositionSensorController() const { return positionSensorController.get(); }
    
    // Debugging
    void printStatus() const;
    String getStatusString() const;
};

#endif // TRAINMANAGER_HPP
