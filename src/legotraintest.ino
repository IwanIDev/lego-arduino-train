#include "Lpf2Hub.h"
#include <Arduino.h>
#include "TrainController.hpp"
#include "BluetoothController.hpp"
#include "InputController.hpp"
#include "LightSensor.hpp"
#include "LightSensorController.hpp"
#include "Action/SensorAction.hpp"
#include "Action/StopAction.hpp"
#include "Action/ReverseAction.hpp"
#include "Action/DelayedAction.hpp"
#include "Action/SpeedAction.hpp"
#include "Action/SequentialAction.hpp"
#include "ActionController.hpp"
#include "ReedSwitchSensor.hpp"
#include "ReedSwitchSensorController.hpp"
#include "PositionTracker.hpp"
#include "PositionAwareSensorController.hpp"
#include "PositionSensorController.hpp"
#include <memory>

Lpf2Hub trainHub;
const byte MOTOR_PORT = (byte)PoweredUpHubPort::B;

const int fastButton = D2;
const int slowButton = D4;

unsigned long previousMillis = 0;
const long speedSwitchInterval = 100;

const int LIGHT_SENSOR_THRESHOLD = 20; // Percentage threshold for light level detection
const int LIGHT_SENSOR_TIMEOUT_THRESHOLD = 500;

BluetoothController bluetoothController(&trainHub);
TrainController trainController(MOTOR_PORT);
InputController inputController(&trainController, fastButton, slowButton);
LightSensor sensors[] = {
    // LightSensor(A0, LIGHT_SENSOR_THRESHOLD, SensorLocation::STATION_STOP, std::unique_ptr<StopAction>(new StopAction(100))),
    // LightSensor(A1, LIGHT_SENSOR_THRESHOLD, SensorLocation::DIRECTION_CHANGE, std::unique_ptr<ReverseAction>(new ReverseAction(0))),
    // LightSensor(A2, LIGHT_SENSOR_THRESHOLD, SensorLocation::SPEED_REDUCE)
};
LightSensorController lightSensorController;
ReedSwitchSensor reedSwitchSensors[] = {
    // Reed switch sensors without actions - actions will be handled by position-based system
    ReedSwitchSensor(D12, SensorLocation::WEST_STATION),
    ReedSwitchSensor(D11, SensorLocation::WEST_TUNNEL),
    ReedSwitchSensor(D10, SensorLocation::EAST_STATION),
    ReedSwitchSensor(D9, SensorLocation::EAST_TUNNEL),
};
ReedSwitchSensorController reedSwitchSensorController;
ActionController actionController(&trainController);

// Position tracking components
PositionTracker positionTracker(SensorLocation::WEST_STATION); // Start at first sensor
PositionAwareSensorController positionAwareSensorController(&reedSwitchSensorController, &lightSensorController, &positionTracker);

// Position-based action controller
PositionSensorController positionSensorController(positionTracker);

// Setup track layout with sensor position relationships
void setupTrackLayout() {
    Serial.println("Setting up track layout...");

    // CURRENT LAYOUT: WEST_STATION <-> WEST_TUNNEL (bidirectional)

    // WEST_STATION
    TrackSegment westStation;
    westStation.location = SensorLocation::WEST_STATION;
    westStation.forwardActions.push_back(std::unique_ptr<SpeedAction>(new SpeedAction(2, 0)));
    // Sequential action for reverse direction: STOP (delay) -> REVERSE (delay) -> SPEED
    {
        std::vector<std::unique_ptr<SensorAction>> reverseActions;
        reverseActions.push_back(std::unique_ptr<SensorAction>(new DelayedAction(
            std::unique_ptr<SensorAction>(new StopAction(0)), 500
        )));
        reverseActions.push_back(std::unique_ptr<SensorAction>(new DelayedAction(
            std::unique_ptr<SensorAction>(new ReverseAction(0)), 500
        )));
        reverseActions.push_back(std::unique_ptr<SensorAction>(new SpeedAction(2, 0)));
        westStation.reverseActions.push_back(std::unique_ptr<SequentialAction>(new SequentialAction(std::move(reverseActions))));
    }
    westStation.nextForward = SensorLocation::WEST_TUNNEL;
    westStation.nextReverse = SensorLocation::WEST_TUNNEL;
    positionTracker.addTrackSegment(westStation);

    // WEST_TUNNEL
    TrackSegment westTunnel;
    westTunnel.location = SensorLocation::WEST_TUNNEL;
    
    // Create SequentialAction for forward direction: Stop -> Reverse -> Speed
    {
        std::vector<std::unique_ptr<SensorAction>> forwardActions;
        forwardActions.push_back(std::unique_ptr<SensorAction>(new StopAction(0)));
        forwardActions.push_back(std::unique_ptr<SensorAction>(new DelayedAction(
            std::unique_ptr<SensorAction>(new ReverseAction(0)), 500
        )));
        forwardActions.push_back(std::unique_ptr<SensorAction>(new SpeedAction(2, 0)));
        westTunnel.forwardActions.push_back(std::unique_ptr<SequentialAction>(new SequentialAction(std::move(forwardActions))));
    }
    
    westTunnel.reverseActions.push_back(std::unique_ptr<SpeedAction>(new SpeedAction(2, 0)));

    westTunnel.nextForward = SensorLocation::EAST_TUNNEL;
    westTunnel.nextReverse = SensorLocation::WEST_STATION;
    positionTracker.addTrackSegment(westTunnel);

}

void printCurrentPosition() {
    Serial.print("Current Position: ");
    Serial.print(getPositionName(positionTracker.getCurrentPosition()));
    Serial.print(" (Previous: ");
    Serial.print(getPositionName(positionTracker.getPreviousPosition()));
    Serial.print(", Direction: ");
    Serial.print(positionTracker.getDirection() == TrainDirection::FORWARD ? "FORWARD" : "REVERSE");
    Serial.println(")");
}

// Helper function to manually set position (useful for testing)
void setManualPosition(SensorLocation position) {
    Serial.print("Manually setting position to: ");
    Serial.println(static_cast<int>(position));
    positionTracker.updatePosition(position);
    printCurrentPosition();
}

// Helper function to get position name for debugging
String getPositionName(SensorLocation location) {
    switch (location) {
        case SensorLocation::WEST_STATION: return "WEST_STATION";
        case SensorLocation::WEST_TUNNEL: return "WEST_TUNNEL";
        case SensorLocation::EAST_STATION: return "EAST_STATION";
        case SensorLocation::EAST_TUNNEL: return "EAST_TUNNEL";
        default: return "UNKNOWN";
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000); // Give time for the serial connection to establish
    Serial.println("Initializing LEGO Train Position Tracking System");
    
    // Setup sensors
    Serial.println("Setting up train controller...");
    for (auto& sensor : reedSwitchSensors) {
        reedSwitchSensorController.addSensor(&sensor);
    }
    Serial.println("Set up train controller");
    
    // Setup track layout for position tracking
    setupTrackLayout();
    
    // Connect the position-based controller to the action controller
    actionController.setPositionController(&positionSensorController);
    Serial.println("Position-based action system enabled");
    
    Serial.println("Position tracking system initialized");
    Serial.print("Starting position: ");
    Serial.println(static_cast<int>(positionTracker.getCurrentPosition()));
}

void loop() {
    unsigned long currentMillis = millis();
    unsigned long deltaT = currentMillis - previousMillis; // Time elapsed between last speed change and now.

    trainController.updateSpeedTimer();
    inputController.handleSerialInput();
    inputController.handleButtonInput(trainController.getState());

    // Check sensors and update position automatically
    if (positionAwareSensorController.checkSensorsAndUpdatePosition()) {
        // Position was updated - execute position-based actions
        SensorLocation currentPos = positionTracker.getCurrentPosition();
        TrainDirection currentDir = positionTracker.getDirection();
        
        Serial.print("Executing actions for position: ");
        Serial.print(getPositionName(currentPos));
        Serial.print(" in direction: ");
        Serial.println(currentDir == TrainDirection::FORWARD ? "FORWARD" : "REVERSE");
        
        // Get and execute position-based actions
        auto actions = positionTracker.getActionsForPosition(currentPos, currentDir);
        Serial.print("Found ");
        Serial.print(actions.size());
        Serial.println(" actions to execute");
        
        for (size_t i = 0; i < actions.size(); i++) {
            auto& action = actions[i];
            if (!action) {
                Serial.print("Action ");
                Serial.print(i);
                Serial.println(" is null, skipping");
                continue;
            }
            
            Serial.print("Processing action ");
            Serial.print(i);
            Serial.print(": ");
            
            if (action->isDelayedAction()) {
                Serial.println("DelayedAction detected");
                DelayedAction* delayedAction = static_cast<DelayedAction*>(action.get());
                actionController.addDelayedAction(delayedAction->createFresh());
                Serial.println("Added DelayedAction to ActionController");
            } else if (action->isSequentialAction()) {
                Serial.println("SequentialAction detected");
                SequentialAction* sequentialAction = static_cast<SequentialAction*>(action.get());
                actionController.addSequentialAction(sequentialAction->createFresh());
                Serial.println("Added SequentialAction to ActionController");
            } else {
                Serial.println("Immediate action detected");
                action->execute(trainController, actionController);
                Serial.println("Executed immediate action");
            }
        }
        
        // Print current position for debugging
        printCurrentPosition();
    }

    actionController.update(); // Update all delayed actions
    
    // Debug: Show active actions
    static unsigned long lastDebugPrint = 0;
    if (currentMillis - lastDebugPrint > 2000) { // Print every 2 seconds
        if (actionController.hasActiveActions()) {
            Serial.print("Active actions: DelayedActions=");
            Serial.print(actionController.getActiveDelayedActionsCount());
            Serial.print(", SequentialActions=");
            Serial.println(actionController.getActiveSequentialActionsCount());
        }
        lastDebugPrint = currentMillis;
    }

    
    // Try to connect to Bluetooth, but don't block the entire loop
    static unsigned long lastBluetoothAttempt = 0;
    const unsigned long bluetoothRetryInterval = 5000; // Retry every 5 seconds
    
    if (currentMillis - lastBluetoothAttempt > bluetoothRetryInterval) {
        if (!bluetoothController.connect()) {
            Serial.println("Bluetooth connection failed - retrying in 5 seconds");
            lastBluetoothAttempt = currentMillis;
            return; // Try again later
        }
        lastBluetoothAttempt = currentMillis;
    }

    // Only proceed with motor control if we have a Bluetooth connection
    if (!bluetoothController.isConnected()) {
        // Print status every 10 seconds to show the system is running
        static unsigned long lastStatusPrint = 0;
        const unsigned long statusPrintInterval = 10000;
        
        if (currentMillis - lastStatusPrint > statusPrintInterval) {
            Serial.println("System running - waiting for Bluetooth connection...");
            lastStatusPrint = currentMillis;
        }
        return;
    }

    if (deltaT < speedSwitchInterval) { // If we haven't reached the interval to change speed we should not change the speed.
        return;
    }

    previousMillis = currentMillis;

    char hubName[] = "trainHub";
    trainHub.setHubName(hubName);

    SPEED currentState = trainController.getState();
    int speed = trainController.getSpeed(currentState);

    if (trainController.hasStateChanged()) {
        trainController.printState();
    }

    bluetoothController.setMotorSpeed(MOTOR_PORT, speed);
}
