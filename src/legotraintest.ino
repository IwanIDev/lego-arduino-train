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
    // Assign different sensor locations to each physical sensor
    ReedSwitchSensor(D12, SensorLocation::WEST_STATION, std::unique_ptr<DelayedAction>(new DelayedAction(std::unique_ptr<SensorAction>(new SpeedAction(-1, 0)), 500))),
    ReedSwitchSensor(D11, SensorLocation::WEST_TUNNEL, []() {
        std::vector<std::unique_ptr<SensorAction>> actions;
        actions.push_back(std::unique_ptr<DelayedAction>(new DelayedAction(std::unique_ptr<SensorAction>(new StopAction(100)), 500)));
        actions.push_back(std::unique_ptr<SensorAction>(new ReverseAction(0)));
        actions.push_back(std::unique_ptr<SensorAction>(new SpeedAction(1, 0)));
        return std::unique_ptr<SequentialAction>(new SequentialAction(std::move(actions)));
    }()),
    ReedSwitchSensor(D10, SensorLocation::EAST_STATION, std::unique_ptr<DelayedAction>(new DelayedAction(std::unique_ptr<SensorAction>(new StopAction(100)), 500))),
    ReedSwitchSensor(D9, SensorLocation::EAST_TUNNEL, std::unique_ptr<DelayedAction>(new DelayedAction(std::unique_ptr<SensorAction>(new StopAction(100)), 500))),
};
ReedSwitchSensorController reedSwitchSensorController;
ActionController actionController(&trainController);

// Position tracking components
PositionTracker positionTracker(SensorLocation::WEST_STATION); // Start at first sensor
PositionAwareSensorController positionAwareSensorController(&reedSwitchSensorController, &lightSensorController, &positionTracker);

// Setup track layout with sensor position relationships
void setupTrackLayout() {
    Serial.println("Setting up track layout...");
    
    // Define the track layout - this should match your physical track
    // Assuming sensors are arranged in sequence: SENSOR_1 -> SENSOR_2 -> SENSOR_3 -> SENSOR_4 -> SENSOR_1 (loop)
    
    // You can customize these based on your actual track layout
    // For now, setting up a simple sequential track
    
    Serial.println("Track layout configured");
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
        // Position was updated - execute sensor actions
        Sensor* triggeredSensor = positionAwareSensorController.getTriggeredSensor();
        if (triggeredSensor) {
            actionController.executeAction(triggeredSensor);
        }
        
        // Print current position for debugging
        printCurrentPosition();
    }

    actionController.update(); // Update all delayed actions

    
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

    trainController.printState();

    bluetoothController.setMotorSpeed(MOTOR_PORT, speed);
}
