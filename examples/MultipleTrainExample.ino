#include "Lpf2Hub.h"
#include <Arduino.h>
#include "TrainManager.hpp"
#include "LightSensor.hpp"
#include "LightSensorController.hpp"
#include "ReedSwitchSensor.hpp"
#include "ReedSwitchSensorController.hpp"
#include "PositionTracker.hpp"
#include "PositionAwareSensorController.hpp"
#include "Action/SensorAction.hpp"
#include "Action/StopAction.hpp"
#include "Action/ReverseAction.hpp"
#include "Action/DelayedAction.hpp"
#include "Action/SpeedAction.hpp"
#include "Action/SequentialAction.hpp"
#include <memory>

// Pin definitions for buttons (one set per train)
const int TRAIN1_FAST_BUTTON = D2;
const int TRAIN1_SLOW_BUTTON = D4;
const int TRAIN2_FAST_BUTTON = D6;
const int TRAIN2_SLOW_BUTTON = D8;

// Motor ports for each train hub
const byte TRAIN1_MOTOR_PORT = (byte)PoweredUpHubPort::B;
const byte TRAIN2_MOTOR_PORT = (byte)PoweredUpHubPort::B;

// Sensor configuration
const int LIGHT_SENSOR_THRESHOLD = 20;
ReedSwitchSensor reedSwitchSensors[] = {
    ReedSwitchSensor(D12, SensorLocation::WEST_STATION),
    ReedSwitchSensor(D11, SensorLocation::WEST_TUNNEL),
    ReedSwitchSensor(D10, SensorLocation::EAST_STATION),
    ReedSwitchSensor(D9, SensorLocation::EAST_TUNNEL),
};

// Sensor controllers
LightSensorController lightSensorController;
ReedSwitchSensorController reedSwitchSensorController;

// Position tracking
PositionTracker positionTracker(SensorLocation::WEST_STATION);
PositionAwareSensorController positionAwareSensorController(&reedSwitchSensorController, 
                                                           &lightSensorController, 
                                                           &positionTracker);

// Train manager
TrainManager trainManager(&positionAwareSensorController, 
                         &positionTracker,
                         &reedSwitchSensorController,
                         &lightSensorController);

// Track layout setup
void setupTrackLayout() {
    Serial.println("Setting up track layout for multiple trains...");

    // WEST_STATION - Stop and reverse
    TrackSegment westStation;
    westStation.location = SensorLocation::WEST_STATION;
    westStation.forwardActions.push_back(std::unique_ptr<SpeedAction>(new SpeedAction(0, 0)));
    
    // Sequential action for reverse direction: STOP -> REVERSE -> SPEED
    {
        std::vector<std::unique_ptr<SensorAction>> reverseActions;
        reverseActions.push_back(std::unique_ptr<SensorAction>(new SpeedAction(-1, 0)));
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

    // WEST_TUNNEL - Reverse direction
    TrackSegment westTunnel;
    westTunnel.location = SensorLocation::WEST_TUNNEL;
    
    // Sequential action for forward direction: Stop -> Reverse -> Speed
    {
        std::vector<std::unique_ptr<SensorAction>> forwardActions;
        forwardActions.push_back(std::unique_ptr<SensorAction>(new StopAction(0)));
        forwardActions.push_back(std::unique_ptr<SensorAction>(new DelayedAction(
            std::unique_ptr<SensorAction>(new ReverseAction(0)), 500
        )));
        forwardActions.push_back(std::unique_ptr<SensorAction>(new SpeedAction(2, 0)));
        westTunnel.forwardActions.push_back(std::unique_ptr<SequentialAction>(new SequentialAction(std::move(forwardActions))));
    }
    
    westTunnel.reverseActions.push_back(std::unique_ptr<SpeedAction>(new SpeedAction(1, 0)));
    westTunnel.nextForward = SensorLocation::WEST_TUNNEL;
    westTunnel.nextReverse = SensorLocation::WEST_STATION;
    positionTracker.addTrackSegment(westTunnel);

    // EAST_STATION - For future expansion
    TrackSegment eastStation;
    eastStation.location = SensorLocation::EAST_STATION;
    eastStation.forwardActions.push_back(std::unique_ptr<SpeedAction>(new SpeedAction(0, 0)));
    eastStation.nextForward = SensorLocation::EAST_TUNNEL;
    eastStation.nextReverse = SensorLocation::EAST_TUNNEL;
    positionTracker.addTrackSegment(eastStation);

    // EAST_TUNNEL - For future expansion
    TrackSegment eastTunnel;
    eastTunnel.location = SensorLocation::EAST_TUNNEL;
    eastTunnel.reverseActions.push_back(std::unique_ptr<SpeedAction>(new SpeedAction(1, 0)));
    eastTunnel.nextForward = SensorLocation::EAST_STATION;
    eastTunnel.nextReverse = SensorLocation::EAST_STATION;
    positionTracker.addTrackSegment(eastTunnel);
}

void printSystemStatus() {
    Serial.println("=== MULTI-TRAIN SYSTEM STATUS ===");
    Serial.print("TrainManager initialized: ");
    Serial.println(trainManager.isInitialized() ? "YES" : "NO");
    Serial.print("Number of trains: ");
    Serial.println(trainManager.getTrainCount());
    
    // Print status of each train
    for (size_t i = 0; i < trainManager.getTrainCount(); i++) {
        TrainInstance* train = trainManager.getTrain(i);
        if (train) {
            train->printStatus();
        }
    }
    
    // Print current position
    Serial.print("Current Position: ");
    Serial.print(getPositionName(positionTracker.getCurrentPosition()));
    Serial.print(" (Direction: ");
    Serial.print(positionTracker.getDirection() == TrainDirection::FORWARD ? "FORWARD" : "REVERSE");
    Serial.println(")");
    Serial.println("================================");
}

String getPositionName(SensorLocation location) {
    switch (location) {
        case SensorLocation::WEST_STATION: return "WEST_STATION";
        case SensorLocation::WEST_TUNNEL: return "WEST_TUNNEL";
        case SensorLocation::EAST_STATION: return "EAST_STATION";
        case SensorLocation::EAST_TUNNEL: return "EAST_TUNNEL";
        default: return "UNKNOWN";
    }
}

void handleSerialCommands() {
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.equalsIgnoreCase("status")) {
            printSystemStatus();
        }
        else if (command.equalsIgnoreCase("stop")) {
            trainManager.stop();
            Serial.println("All trains stopped");
        }
        else if (command.equalsIgnoreCase("emergency")) {
            trainManager.emergencyStop();
            Serial.println("EMERGENCY STOP executed");
        }
        else if (command.startsWith("speed")) {
            // Parse command like "speed train1 2" or "speed train2 0"
            int firstSpace = command.indexOf(' ');
            int secondSpace = command.indexOf(' ', firstSpace + 1);
            
            if (firstSpace > 0 && secondSpace > 0) {
                String trainName = command.substring(firstSpace + 1, secondSpace);
                int speed = command.substring(secondSpace + 1).toInt();
                
                TrainInstance* train = trainManager.getTrain(trainName);
                if (train) {
                    auto* trainController = train->getTrainController();
                    if (trainController) {
                        trainController->setState(speed > 0 ? SPEED::GO : SPEED::STOPPED);
                        Serial.print("Set ");
                        Serial.print(trainName);
                        Serial.print(" speed to ");
                        Serial.println(speed);
                    }
                } else {
                    Serial.print("Train not found: ");
                    Serial.println(trainName);
                }
            }
        }
        else if (command.startsWith("reverse")) {
            // Parse command like "reverse train1" or "reverse train2"
            int spaceIndex = command.indexOf(' ');
            if (spaceIndex > 0) {
                String trainName = command.substring(spaceIndex + 1);
                TrainInstance* train = trainManager.getTrain(trainName);
                if (train) {
                    auto* trainController = train->getTrainController();
                    if (trainController) {
                        bool currentReverse = trainController->getReverse();
                        trainController->setReverse(!currentReverse);
                        Serial.print("Toggled reverse for ");
                        Serial.print(trainName);
                        Serial.print(" to ");
                        Serial.println(!currentReverse ? "ON" : "OFF");
                    }
                } else {
                    Serial.print("Train not found: ");
                    Serial.println(trainName);
                }
            }
        }
        else {
            Serial.println("Available commands:");
            Serial.println("  status - Show system status");
            Serial.println("  stop - Stop all trains");
            Serial.println("  emergency - Emergency stop");
            Serial.println("  speed <trainName> <speed> - Set train speed");
            Serial.println("  reverse <trainName> - Toggle train reverse");
        }
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
    train1Config.motorPort = TRAIN1_MOTOR_PORT;
    train1Config.fastButtonPin = TRAIN1_FAST_BUTTON;
    train1Config.slowButtonPin = TRAIN1_SLOW_BUTTON;
    size_t train1Index = trainManager.addTrain(train1Config);
    
    TrainConfig train2Config;
    train2Config.hubName = "Train2";
    train2Config.motorPort = TRAIN2_MOTOR_PORT;
    train2Config.fastButtonPin = TRAIN2_FAST_BUTTON;
    train2Config.slowButtonPin = TRAIN2_SLOW_BUTTON;
    size_t train2Index = trainManager.addTrain(train2Config);
    
    // Initialize the train manager
    if (trainManager.initialize()) {
        Serial.println("Multi-train system initialized successfully");
    } else {
        Serial.println("Failed to initialize multi-train system");
    }
    
    // Print initial status
    printSystemStatus();
    
    Serial.println("\nSystem ready. Use serial commands to control trains.");
    Serial.println("Type 'help' for available commands.");
}

void loop() {
    // Update the train manager (handles all trains)
    trainManager.update();
    
    // Handle serial commands
    handleSerialCommands();
    
    // Print status periodically
    static unsigned long lastStatusPrint = 0;
    unsigned long currentTime = millis();
    if (currentTime - lastStatusPrint > 10000) { // Every 10 seconds
        printSystemStatus();
        lastStatusPrint = currentTime;
    }
    
    // Small delay to prevent overwhelming the system
    delay(10);
}
