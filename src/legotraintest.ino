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
#include "Train/TrainInstance.hpp"
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

// Position aware sensor controller (no longer needs a position tracker)
PositionAwareSensorController positionAwareSensorController(&reedSwitchSensorController, &lightSensorController);

// Train Manager
TrainManager trainManager(&positionAwareSensorController, &reedSwitchSensorController, &lightSensorController);

// Setup track layout with sensor position relationships for a specific position tracker
void setupTrackLayoutForTracker(PositionTracker& positionTracker) {
    Serial.println("Setting up track layout for position tracker...");

    const int SPEED_WEST_STATION = 3;
    const int SPEED_WEST_TUNNEL = 3;

    // CURRENT LAYOUT: WEST_STATION <-> WEST_TUNNEL (bidirectional)

    // WEST_STATION
    TrackSegment westStation;
    westStation.location = SensorLocation::WEST_STATION;
    // No forward actions needed for WEST_STATION - train just passes through
    
    // Sequential action for reverse direction: STOP (delay) -> REVERSE (delay) -> SPEED
    {
        std::vector<std::unique_ptr<SensorAction>> reverseActions;
        reverseActions.push_back(std::unique_ptr<SensorAction>(
            new SpeedAction(-1, 0)
        ));
        reverseActions.push_back(std::unique_ptr<SensorAction>(new DelayedAction(
            std::unique_ptr<SensorAction>(new StopAction(0)), 1500
        )));
        reverseActions.push_back(std::unique_ptr<SensorAction>(new DelayedAction(
            std::unique_ptr<SensorAction>(new ReverseAction(0)), 500
        )));
        reverseActions.push_back(std::unique_ptr<SensorAction>(new SpeedAction(SPEED_WEST_TUNNEL, 0)));
        westStation.reverseActions.push_back(std::unique_ptr<SequentialAction>(new SequentialAction(std::move(reverseActions))));
    }
    westStation.nextForward = SensorLocation::WEST_TUNNEL;
    westStation.nextReverse = SensorLocation::WEST_TUNNEL;
    positionTracker.addTrackSegment(westStation);

    // EAST_STATION
    TrackSegment eastStation;
    eastStation.location = SensorLocation::EAST_STATION;

    // Create SequentialAction for reverse direction: Stop -> Reverse -> Speed
    {
        std::vector<std::unique_ptr<SensorAction>> reverseActions;
        reverseActions.push_back(std::unique_ptr<SensorAction>(new StopAction(0)));
        reverseActions.push_back(std::unique_ptr<SensorAction>(new DelayedAction(
            std::unique_ptr<SensorAction>(new ReverseAction(0)), 500
        )));
        reverseActions.push_back(std::unique_ptr<SensorAction>(new SpeedAction(SPEED_WEST_TUNNEL, 0)));
        eastStation.reverseActions.push_back(std::unique_ptr<SequentialAction>(new SequentialAction(std::move(reverseActions))));
    }

    eastStation.forwardActions.push_back(std::unique_ptr<SpeedAction>(new SpeedAction(0, 0)));

    eastStation.nextForward = SensorLocation::EAST_TUNNEL;
    eastStation.nextReverse = SensorLocation::EAST_TUNNEL;
    positionTracker.addTrackSegment(eastStation);

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
        forwardActions.push_back(std::unique_ptr<SensorAction>(new SpeedAction(SPEED_WEST_STATION, 0)));
        westTunnel.forwardActions.push_back(std::unique_ptr<SequentialAction>(new SequentialAction(std::move(forwardActions))));
    }

    westTunnel.reverseActions.push_back(std::unique_ptr<SpeedAction>(new SpeedAction(0, 0)));

    westTunnel.nextForward = SensorLocation::WEST_TUNNEL;
    westTunnel.nextReverse = SensorLocation::WEST_STATION;
    positionTracker.addTrackSegment(westTunnel);

    // EAST_TUNNEL
    TrackSegment eastTunnel;
    eastTunnel.location = SensorLocation::EAST_TUNNEL;

    // Create SequentialAction for forward direction: Stop -> Reverse -> Speed
    {
        std::vector<std::unique_ptr<SensorAction>> forwardActions;
        forwardActions.push_back(std::unique_ptr<SensorAction>(new StopAction(0)));
        forwardActions.push_back(std::unique_ptr<SensorAction>(new DelayedAction(
            std::unique_ptr<SensorAction>(new ReverseAction(0)), 500
        )));
        forwardActions.push_back(std::unique_ptr<SensorAction>(new SpeedAction(SPEED_WEST_STATION, 0)));
        eastTunnel.forwardActions.push_back(std::unique_ptr<SequentialAction>(new SequentialAction(std::move(forwardActions))));
    }

    eastTunnel.reverseActions.push_back(std::unique_ptr<SpeedAction>(new SpeedAction(0, 0)));

    eastTunnel.nextForward = SensorLocation::EAST_TUNNEL;
    eastTunnel.nextReverse = SensorLocation::EAST_STATION;
    positionTracker.addTrackSegment(eastTunnel);
}

// Legacy function maintained for compatibility
void setupTrackLayout() {
    // This function is now empty since individual trains handle their own track layout
}

void printCurrentPosition() {
    Serial.println("=== Train Positions ===");
    for (size_t i = 0; i < trainManager.getTrainCount(); i++) {
        TrainInstance* train = trainManager.getTrain(i);
        if (train && train->getPositionTracker()) {
            PositionTracker* tracker = train->getPositionTracker();
            Serial.print("Train ");
            Serial.print(i);
            Serial.print(" (");
            Serial.print(train->getHubName());
            Serial.print("): Position: ");
            Serial.print(getPositionName(tracker->getCurrentPosition()));
            Serial.print(" (Previous: ");
            Serial.print(getPositionName(tracker->getPreviousPosition()));
            Serial.print(", Direction: ");
            Serial.print(tracker->getDirection() == TrainDirection::FORWARD ? "FORWARD" : "REVERSE");
            Serial.println(")");
        }
    }
    Serial.println("======================");
}

// Helper function to manually set position for a specific train (useful for testing)
void setManualPosition(size_t trainIndex, SensorLocation position) {
    TrainInstance* train = trainManager.getTrain(trainIndex);
    if (train && train->getPositionTracker()) {
        Serial.print("Manually setting train ");
        Serial.print(trainIndex);
        Serial.print(" position to: ");
        Serial.println(static_cast<int>(position));
        train->getPositionTracker()->updatePosition(position);
        printCurrentPosition();
    } else {
        Serial.print("Invalid train index: ");
        Serial.println(trainIndex);
    }
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
    
    // Attach interrupts to sensor pins for more reliable detection
    // attachInterrupt(digitalPinToInterrupt(D12), sensorISR, CHANGE);
    // attachInterrupt(digitalPinToInterrupt(D11), sensorISR, CHANGE);
    // attachInterrupt(digitalPinToInterrupt(D10), sensorISR, CHANGE);
    // attachInterrupt(digitalPinToInterrupt(D9), sensorISR, CHANGE);
    // Serial.println("Sensor interrupts attached to pins D12, D11, D10, D9");
    
    // Setup track layout
    setupTrackLayout();
    
    // Add trains to the manager
    TrainConfig train1Config;
    train1Config.hubName = "Train1";
    train1Config.motorPort = MOTOR_PORT;
    train1Config.fastButtonPin = fastButton;
    train1Config.slowButtonPin = slowButton;
    train1Config.initialPosition = SensorLocation::WEST_STATION;
    size_t train1Index = trainManager.addTrain(train1Config);
    
    TrainConfig train2Config;
    train2Config.hubName = "Train2";
    train2Config.motorPort = MOTOR_PORT;
    train2Config.fastButtonPin = fastButton;
    train2Config.slowButtonPin = slowButton;
    train2Config.initialPosition = SensorLocation::EAST_STATION;
    size_t train2Index = trainManager.addTrain(train2Config);
    
    // Setup track layout for each train's position tracker
    TrainInstance* train1 = trainManager.getTrain(train1Index);
    if (train1 && train1->getPositionTracker()) {
        setupTrackLayoutForTracker(*train1->getPositionTracker());
        Serial.println("Track layout configured for Train1");
    }
    
    TrainInstance* train2 = trainManager.getTrain(train2Index);
    if (train2 && train2->getPositionTracker()) {
        setupTrackLayoutForTracker(*train2->getPositionTracker());
        Serial.println("Track layout configured for Train2");
    }
    
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
    if (currentTime - lastStatusPrint > 5000) { // Every 5 seconds
        printCurrentPosition();
        lastStatusPrint = currentTime;
    }

    // Debug: Periodically check raw pin states
    static unsigned long lastPinCheck = 0;
    if (currentTime - lastPinCheck > 2000) { // Every 2 seconds for more frequent monitoring
        Serial.print("PIN STATE CHECK - D12: ");
        Serial.print(digitalRead(D12) == LOW ? "ACTIVE" : "INACTIVE");
        Serial.print(", D11: ");
        Serial.print(digitalRead(D11) == LOW ? "ACTIVE" : "INACTIVE");
        Serial.print(", D10: ");
        Serial.print(digitalRead(D10) == LOW ? "ACTIVE" : "INACTIVE");
        Serial.print(", D9: ");
        Serial.println(digitalRead(D9) == LOW ? "ACTIVE" : "INACTIVE");
        lastPinCheck = currentTime;
    }
    
    // Reduced delay for more responsive sensor polling
    delay(1);
}
