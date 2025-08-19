#ifndef TRAININSTANCE_HPP
#define TRAININSTANCE_HPP

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
#include "Train/TrainManager.hpp"
#include <vector>
#include <memory>
#include <map>

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
    unsigned long connectionTimestamp;
    unsigned long lastCommandTimestamp;
    bool firstCommandSent;
    
public:
    TrainInstance(const TrainConfig& trainConfig, size_t id, PositionTracker* positionTracker);
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
    
    // Debugging
    void printStatus() const;
    String getStatusString() const;
};

#endif // TRAININSTANCE_HPP