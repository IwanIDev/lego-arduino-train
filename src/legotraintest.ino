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
#include "Train/TrainManager.hpp"
#include <memory>

Lpf2Hub trainHub;
const byte MOTOR_PORT = (byte)PoweredUpHubPort::B;

const int fastButton = D2;
const int slowButton = D4;

unsigned long previousMillis = 0;
const long speedSwitchInterval = 100;

const int LIGHT_SENSOR_THRESHOLD = 20; // Percentage threshold for light level detection
const int LIGHT_SENSOR_TIMEOUT_THRESHOLD = 500;

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

// Position tracking components
PositionTracker positionTracker(SensorLocation::WEST_STATION); // Start at first sensor
PositionAwareSensorController positionAwareSensorController(&reedSwitchSensorController, &lightSensorController, &positionTracker);

// Position-based action controller
PositionSensorController positionSensorController(positionTracker);

// Train Manager
TrainManager trainManager(&positionAwareSensorController, &positionTracker, &reedSwitchSensorController, &lightSensorController);

// Setup track layout with sensor position relationships
void setupTrackLayout() {
    Serial.println("Setting up track layout...");

    // CURRENT LAYOUT: WEST_STATION <-> WEST_TUNNEL (bidirectional)

    // WEST_STATION
    TrackSegment westStation;
    westStation.location = SensorLocation::WEST_STATION;
    // No forward actions needed for WEST_STATION - train just passes through
    
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

    westTunnel.reverseActions.push_back(std::unique_ptr<SpeedAction>(new SpeedAction(0, 0)));

    westTunnel.nextForward = SensorLocation::WEST_TUNNEL;
    westTunnel.nextReverse = SensorLocation::WEST_STATION;
    positionTracker.addTrackSegment(westTunnel);

}

void printCurrentPosition() {
    Serial.print("Current Position: ");
    Serial.print(getPositionName(positionTracker.getCurrentPosition()));
    Serial.print(" (Previous: ");
    Serial.print(getPositionName(positionTracker.getPreviousPosition()));
    Serial.print(", PositionTracker Direction: ");
    Serial.print(positionTracker.getDirection() == TrainDirection::FORWARD ? "FORWARD" : "REVERSE");
    Serial.println();
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
    delay(1000);
    Serial.println("Initializing Multi-Train LEGO System");
    
    // Setup sensors
    for (auto& sensor : reedSwitchSensors) {
        reedSwitchSensorController.addSensor(&sensor);
    }
    
    // Setup track layout
    setupTrackLayout();
    
    // Add trains to the manager
    TrainConfig train1Config;
    train1Config.hubName = "Train1";
    train1Config.motorPort = MOTOR_PORT;
    train1Config.fastButtonPin = fastButton;
    train1Config.slowButtonPin = slowButton;
    size_t train1Index = trainManager.addTrain(train1Config);
    
    TrainConfig train2Config;
    train2Config.hubName = "Train2";
    train2Config.motorPort = MOTOR_PORT;
    train2Config.fastButtonPin = fastButton;
    train2Config.slowButtonPin = slowButton;
    size_t train2Index = trainManager.addTrain(train2Config);
    
    // Initialize the train manager
    if (trainManager.initialize()) {
        Serial.println("Multi-train system initialized successfully");
    } else {
        Serial.println("Failed to initialize multi-train system");
    }
    
    // Print initial status
    printCurrentPosition();
    
    Serial.println("\nSystem ready. Use serial commands to control trains.");
    Serial.println("Type 'help' for available commands.");
}

void loop() {
    // Update the train manager (handles all trains)
    trainManager.update();
    
    // Print status periodically
    static unsigned long lastStatusPrint = 0;
    unsigned long currentTime = millis();
    if (currentTime - lastStatusPrint > 10000) { // Every 10 seconds
        printCurrentPosition();
        lastStatusPrint = currentTime;
    }
    
    // Small delay to prevent overwhelming the system
    delay(10);
}
