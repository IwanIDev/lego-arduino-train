#ifndef LEGO_TRAIN_CONTROLLER_H
#define LEGO_TRAIN_CONTROLLER_H

#include <Arduino.h>
#include <vector>
#include <memory>
#include "Lpf2Hub.h"

// Include all necessary headers from the library
#include "Controllers/TrainController.h"
#include "Controllers/BluetoothController.h"
#include "Controllers/InputController.h"
#include "Controllers/ActionController.h"
#include "Sensors/LightSensor.h"
#include "Sensors/LightSensorController.h"
#include "Sensors/ReedSwitchSensor.h"
#include "Sensors/ReedSwitchSensorController.h"
#include "Sensors/PositionAwareSensorController.h"
#include "Sensors/PositionSensorController.h"
#include "Position/PositionTracker.h"
#include "Position/SensorLocation.h"
#include "Train/TrainManager.h"
#include "Train/TrainInstance.h"
#include "Switch/SwitchController.h"
#include "Actions/SensorAction.h"
#include "Actions/StopAction.h"
#include "Actions/ReverseAction.h"
#include "Actions/DelayedAction.h"
#include "Actions/SpeedAction.h"
#include "Actions/SequentialAction.h"
#include "Actions/WaitForPositionAction.h"
#include "Actions/SwitchAction.h"

// Enum for action types to simplify API
enum class TrainActionType {
    STOP,
    REVERSE,
    SPEED,
    SWITCH,
    DELAY
};

// Simplified action configuration struct
struct ActionConfig {
    TrainActionType type;
    int speed = 0;
    int targetSpeed = 0;
    int switchId = 0;
    int switchPosition = 0;
    int delayMs = 0;
    std::unique_ptr<ActionConfig> delayedAction = nullptr;
    
    ActionConfig(TrainActionType t) : type(t) {}
};

// Note: No predefined sensor locations - create your own SensorLocation objects
// Example: SensorLocation myStation("MY_STATION", 1);

// Switch position constants
namespace SwitchPositions {
    const int STRAIGHT = static_cast<int>(SwitchPosition::STRAIGHT);
    const int DIVERGED = static_cast<int>(SwitchPosition::DIVERGED);
}

/**
 * Main Arduino library interface for LEGO Train Controller
 * 
 * This class provides a simplified API for controlling LEGO Powered Up trains
 * with automated layouts including sensors, switches, and position tracking.
 */
class LegoTrainController {
private:
    LightSensorController lightSensorController;
    ReedSwitchSensorController reedSwitchSensorController;
    PositionAwareSensorController positionAwareSensorController;
    TrainManager trainManager;
    SwitchController switchController;
    bool initialized;
    
    // Helper function to create actions from configuration
    std::unique_ptr<SensorAction> createActionFromConfig(const ActionConfig& config);
    
public:
    /**
     * Constructor - initializes all controllers
     */
    LegoTrainController();
    
    /**
     * Destructor
     */
    ~LegoTrainController();
    
    /**
     * Initialize the train controller system
     * Call this in Arduino setup()
     * @return true if initialization successful
     */
    bool begin();
    
    /**
     * Main update loop - call this in Arduino loop()
     */
    void update();
    
    // ===== Train Management =====
    
    /**
     * Add a train to the system
     * @param hubName Name of the Powered Up hub
     * @param motorPort Motor port (typically PoweredUpHubPort::A or B)
     * @param initialPosition Initial sensor location
     * @return Index of the added train
     */
    size_t addTrain(const String& hubName, byte motorPort, const SensorLocation& initialPosition);
    
    /**
     * Add a train with full configuration
     * @param config TrainConfig struct with all parameters
     * @return Index of the added train
     */
    size_t addTrain(const TrainConfig& config);
    
    /**
     * Get the number of trains in the system
     * @return Number of trains
     */
    size_t getTrainCount();
    
    // ===== Sensor Management =====
    
    /**
     * Add a light sensor
     * @param pin Analog pin number
     * @param threshold Light threshold (0-100)
     * @param location Sensor location
     * @return true if added successfully
     */
    bool addLightSensor(int pin, int threshold, const SensorLocation& location);
    
    /**
     * Add a reed switch sensor
     * @param pin Digital pin number
     * @param location Sensor location
     * @return true if added successfully
     */
    bool addReedSwitchSensor(int pin, const SensorLocation& location);
    
    // ===== Switch Management =====
    
    /**
     * Add a track switch
     * @param relayPin Digital pin connected to switch relay
     * @param initialPosition Initial position (STRAIGHT or DIVERGED)
     * @return Switch ID for later reference
     */
    int addSwitch(int relayPin, int initialPosition = SwitchPositions::STRAIGHT);
    
    /**
     * Operate a switch
     * @param switchId Switch ID returned by addSwitch()
     * @param position New position (STRAIGHT or DIVERGED)
     * @return true if operation successful
     */
    bool operateSwitch(int switchId, int position);
    
    // ===== Train Control =====
    
    /**
     * Set train speed
     * @param trainIndex Train index from addTrain()
     * @param speed Speed value (-100 to 100, negative for reverse)
     * @return true if successful
     */
    bool setTrainSpeed(size_t trainIndex, int speed);
    
    /**
     * Stop a train
     * @param trainIndex Train index from addTrain()
     * @return true if successful
     */
    bool stopTrain(size_t trainIndex);
    
    /**
     * Reverse a train's direction
     * @param trainIndex Train index from addTrain()
     * @return true if successful
     */
    bool reverseTrain(size_t trainIndex);
    
    // ===== Track Layout =====
    
    /**
     * Add a track segment to define track topology
     * @param location Current sensor location
     * @param nextForward Next location when moving forward
     * @param nextReverse Next location when moving in reverse
     * @param trainIndex Train index (default: 0)
     */
    void addTrackSegment(const SensorLocation& location, const SensorLocation& nextForward, const SensorLocation& nextReverse, size_t trainIndex = 0);
    
    // ===== Action System =====
    
    /**
     * Add a stop action at a sensor location
     * @param location Sensor location
     * @param reverse true if reverse direction, false if forwards
     * @param trainIndex Train index
     * @param speed Speed when stopping (usually 0)
     */
    void addStopAction(const SensorLocation& location, const bool reverse = false, size_t trainIndex, int speed = 0);
    
    /**
     * Add a reverse action at a sensor location
     * @param location Sensor location
     * @param reverse true if reverse direction, false if forwards
     * @param trainIndex Train index
     * @param speed Speed when reversing (usually 0)
     */
    void addReverseAction(const SensorLocation& location, const bool reverse = false, size_t trainIndex, int speed = 0);

    /**
     * Add a speed change action at a sensor location
     * @param location Sensor location
     * @param reverse true if reverse direction, false if forwards
     * @param trainIndex Train index
     * @param speed Current speed during action
     * @param targetSpeed New speed to set
     */
    void addSpeedAction(const SensorLocation& location, const bool reverse = false, size_t trainIndex, int speed = 0, int targetSpeed);
    
    /**
     * Add a switch operation action at a sensor location
     * @param location Sensor location
     * @param reverse true if reverse direction, false if forwards
     * @param trainIndex Train index
     * @param switchId Switch ID
     * @param position Switch position (STRAIGHT or DIVERGED)
     * @param speed Speed during switch operation
     */
    void addSwitchAction(const SensorLocation& location, const bool reverse = false, size_t trainIndex, int switchId, int position, int speed = 0);
    
    /**
     * Add a sequence of actions at a sensor location
     * @param location Sensor location
     * @param reverse true if reverse direction, false if forwards
     * @param trainIndex Train index
     * @param actionConfigs Vector of action configurations
     */
    void addSequentialAction(const SensorLocation& location, const bool reverse = false, size_t trainIndex, const std::vector<ActionConfig>& actionConfigs);
    
    // ===== Status and Debugging =====
    
    /**
     * Get current train position
     * @param trainIndex Train index
     * @return Current sensor location (-1 if unknown)
     */
    int getTrainPosition(size_t trainIndex);
    
    /**
     * Get current train speed
     * @param trainIndex Train index
     * @return Current speed
     */
    int getTrainSpeed(size_t trainIndex);
    
    /**
     * Check if train is connected via Bluetooth
     * @param trainIndex Train index
     * @return true if connected
     */
    bool isTrainConnected(size_t trainIndex);
    
    /**
     * Enable or disable debug output
     * @param enable true to enable debug output
     */
    void enableDebug(bool enable = true);
    
    /**
     * Print current system status to Serial
     */
    void printStatus();
};

#endif // LEGO_TRAIN_CONTROLLER_H