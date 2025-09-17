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
#include "Action/WaitForPositionAction.hpp"
#include "Action/SwitchAction.hpp"
#include "ActionController.hpp"
#include "ReedSwitchSensor.hpp"
#include "ReedSwitchSensorController.hpp"
#include "PositionTracker.hpp"
#include "PositionAwareSensorController.hpp"
#include "PositionSensorController.hpp"
#include "Train/TrainManager.hpp"
#include "Train/TrainInstance.hpp"
#include "Switch/SwitchController.hpp"
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
    // Examples using custom sensor locations:
    // LightSensor(A0, LIGHT_SENSOR_THRESHOLD, SensorLocation("STATION_STOP", 10), std::unique_ptr<StopAction>(new StopAction(100))),
    // LightSensor(A1, LIGHT_SENSOR_THRESHOLD, SensorLocation("DIRECTION_CHANGE", 11), std::unique_ptr<ReverseAction>(new ReverseAction(0))),
    // LightSensor(A2, LIGHT_SENSOR_THRESHOLD, SensorLocation("SPEED_REDUCE", 12))
};
LightSensorController lightSensorController;
ReedSwitchSensor reedSwitchSensors[] = {
    // Reed switch sensors without actions - actions will be handled by position-based system
    ReedSwitchSensor(D12, SensorLocation::createWestStation()),
    ReedSwitchSensor(D11, SensorLocation::createWestTunnel()),
    ReedSwitchSensor(D10, SensorLocation::createEastStation()),
    ReedSwitchSensor(D9, SensorLocation::createEastTunnel()),
};
ReedSwitchSensorController reedSwitchSensorController;

// Position aware sensor controller (no longer needs a position tracker)
PositionAwareSensorController positionAwareSensorController(&reedSwitchSensorController, &lightSensorController);

// Train Manager
TrainManager trainManager(&positionAwareSensorController, &reedSwitchSensorController, &lightSensorController);

// Switch controller for managing track switches
SwitchController switchController;
unsigned int switchRelayPins[] = {D5, D6}; // Example relay pins for switches
unsigned int switchIds[sizeof(switchRelayPins) / sizeof(switchRelayPins[0])] = {1, 2};

// Setup track layout with sensor position relationships for a specific position tracker
void setupTrackLayoutForTracker(PositionTracker& positionTracker, TrainInstance& instance) {
    Serial.println("Setting up track layout...");
    // Define track segments based on the physical layout
    // WEST_STATION
    TrackSegment westStation;
    westStation.location = SensorLocation::createWestStation();
    westStation.nextForward = SensorLocation::createWestTunnel();
    westStation.nextReverse = SensorLocation::createWestTunnel();
    positionTracker.addTrackSegment(westStation);

    // WEST_TUNNEL
    TrackSegment westTunnel;
    westTunnel.location = SensorLocation::createWestTunnel();
    westTunnel.nextForward = SensorLocation::createWestStation();
    westTunnel.nextReverse = SensorLocation::createEastTunnel();

    // Forward Action at WEST_TUNNEL: STOP -> SWITCH -> REVERSE -> SPEED
    {
        std::vector<std::unique_ptr<SensorAction>> actions;
        actions.push_back(std::unique_ptr<SensorAction>(new StopAction(0)));
        actions.push_back(std::unique_ptr<SensorAction>(new SwitchAction(switchIds[0], SwitchPosition::DIVERGED, 0, &switchController)));
        actions.push_back(std::unique_ptr<SensorAction>(new SwitchAction(switchIds[1], SwitchPosition::DIVERGED, 0, &switchController)));
        actions.push_back(std::unique_ptr<SensorAction>(new ReverseAction(0)));
        actions.push_back(std::unique_ptr<SensorAction>(new DelayedAction(std::unique_ptr<SensorAction>(new SpeedAction(5, 0)), 500)));
        westTunnel.forwardActions.push_back(std::unique_ptr<SensorAction>(new SequentialAction(std::move(actions))));
    }
    positionTracker.addTrackSegment(westTunnel);

    // EAST_TUNNEL
    TrackSegment eastTunnel;
    eastTunnel.location = SensorLocation::createEastTunnel();
    eastTunnel.nextForward = SensorLocation::createWestTunnel();
    eastTunnel.nextReverse = SensorLocation::createEastStation();

    // Reverse Action at EAST_TUNNEL: STOP -> REVERSE -> SPEED
    {
        std::vector<std::unique_ptr<SensorAction>> actions;
        actions.push_back(std::unique_ptr<SensorAction>(new StopAction(0)));
        actions.push_back(std::unique_ptr<SensorAction>(new ReverseAction(0)));
        actions.push_back(std::unique_ptr<SensorAction>(new SpeedAction(5, 0)));
        eastTunnel.reverseActions.push_back(std::unique_ptr<SensorAction>(new SequentialAction(std::move(actions))));
    }
    positionTracker.addTrackSegment(eastTunnel);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Initializing Multi-Train LEGO System");
    
    // Setup sensors
    for (auto& sensor : reedSwitchSensors) {
        reedSwitchSensorController.addSensor(&sensor);
    }

    // Setup switches
    for (size_t i = 0; i < sizeof(switchRelayPins) / sizeof(switchRelayPins[0]); ++i) {
        int switchId = switchController.addSwitch(switchRelayPins[i], SwitchPosition::STRAIGHT);
        switchIds[i] = switchId; // Store the generated switch ID
        Serial.print("Added switch with ID: ");
        Serial.println(switchId);
    }
        
    // Add trains to the manager
    TrainConfig train1Config;
    train1Config.hubName = "Train1";
    train1Config.motorPort = MOTOR_PORT;
    train1Config.fastButtonPin = fastButton;
    train1Config.slowButtonPin = slowButton;
    train1Config.initialPosition = SensorLocation::createWestStation();
    size_t train1Index = trainManager.addTrain(train1Config);
    
    // Setup track layout for each train's position tracker
    TrainInstance* train1 = trainManager.getTrain(train1Index);
    if (train1 && train1->getPositionTracker()) {
        setupTrackLayoutForTracker(*train1->getPositionTracker(), *train1);
        Serial.println("Track layout configured for Train1");
    }

    // Initialize the train manager
    if (trainManager.initialize()) {
        Serial.println("Multi-train system initialized successfully");
    } else {
        Serial.println("Failed to initialize multi-train system");
    }
    
    Serial.println("\nSystem ready. Use serial commands to control trains.");
    Serial.println("Type 'help' for available commands.");
}

void loop() {
    // Update the train manager (handles all trains)
    trainManager.update();
}
