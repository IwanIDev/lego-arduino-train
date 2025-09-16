#ifndef TRAIN_INSTANCE_H
#define TRAIN_INSTANCE_H

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

// Forward declaration to avoid circular dependency
class TrainManager;

// TrainConfig struct (if not already defined elsewhere)
struct TrainConfig;

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
    std::unique_ptr<PositionTracker> positionTracker;
    
    // Configuration
    TrainConfig config;
    String hubName;
    size_t instanceId;
    
    // State
    bool connected;
    unsigned long lastConnectionAttempt;
    unsigned long connectionTimestamp;
    unsigned long lastCommandTimestamp;
    bool firstCommandSent;
    
public:
    TrainInstance(const TrainConfig& trainConfig, size_t id);
    ~TrainInstance();
    
    // Initialization
    bool initialize();
    
    // Connection management
    bool connect();
    bool isConnected() const { return connected && bluetoothController->isConnected(); }
    bool validateConnection();
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
    PositionTracker* getPositionTracker() const { return positionTracker.get(); }
    
    // Debugging
    void printStatus() const;
    String getStatusString() const;
};

#endif // TRAIN_INSTANCE_H