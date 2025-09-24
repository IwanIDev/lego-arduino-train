/*
 * ConditionalPositionExample - Advanced LEGO Train Controller Example
 * 
 * This example demonstrates how to use conditional position-based actions to create
 * intelligent routing decisions based on where the train came from.
 * 
 * Scenario:
 * - Train passes over sensor "JUNCTION" from two different paths
 * - If coming from "STATION_A", speed up to 30 and take the straight track
 * - If coming from "STATION_B", slow down to 10 and take the diverged track
 * 
 * Hardware required:
 * - Arduino ESP32 or compatible
 * - LEGO Powered Up train with hub
 * - Light sensors at track positions
 * - Relay module for switch control
 * 
 * Created by IwanIDev
 */

#include <LegoTrainController.h>

// Create the main controller instance
LegoTrainController trainController;

// Train configuration
const String TRAIN_HUB_NAME = "MyTrain";
const byte MOTOR_PORT = (byte)PoweredUpHubPort::A;

// Sensor pins
const int START_SENSOR_PIN = A0;
const int STATION_A_SENSOR_PIN = A1;
const int STATION_B_SENSOR_PIN = A2;
const int JUNCTION_SENSOR_PIN = A3;

// Switch control pin
const int SWITCH_CONTROL_PIN = 7;

// Sensor threshold for light sensors
const int SENSOR_THRESHOLD = 50;

// Create custom sensor locations
SensorLocation startPosition("START", 1);
SensorLocation stationA("STATION_A", 2);
SensorLocation stationB("STATION_B", 3);
SensorLocation junction("JUNCTION", 4);

// Train index
size_t trainIndex;

// Switch ID
int switchId;
void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("=== LEGO Train Controller - Conditional Position Example ===");
  
  // Initialize the library
  if (!trainController.begin()) {
    Serial.println("Failed to initialize train controller!");
    return;
  }
  
  // Add a train to the system
  trainIndex = trainController.addTrain(TRAIN_HUB_NAME, MOTOR_PORT, startPosition);
  Serial.print("Added train with index: ");
  Serial.println(trainIndex);
  
  // Add light sensors at key positions
  trainController.addLightSensor(START_SENSOR_PIN, SENSOR_THRESHOLD, startPosition);
  trainController.addLightSensor(STATION_A_SENSOR_PIN, SENSOR_THRESHOLD, stationA);
  trainController.addLightSensor(STATION_B_SENSOR_PIN, SENSOR_THRESHOLD, stationB);
  trainController.addLightSensor(JUNCTION_SENSOR_PIN, SENSOR_THRESHOLD, junction);
  
  // Add a switch at the junction
  switchId = trainController.addSwitch(SWITCH_CONTROL_PIN, SwitchPositions::STRAIGHT);
  Serial.print("Added switch with ID: ");
  Serial.println(switchId);
  
  // Setup the conditional position action
  setupConditionalActions();
  
  // Enable debug output
  trainController.enableDebug(true);
  
  Serial.println("Setup complete! Train controller ready.");
  Serial.println("The train will now make routing decisions based on its previous position:");
  Serial.println("  Coming from STATION_A -> Speed up and go straight");
  Serial.println("  Coming from STATION_B -> Slow down and diverge");
  Serial.println();
}

void setupConditionalActions() {
  // Create actions for when train comes from STATION_A
  // SpeedAction: set speed to 30 with 10ms delay
  auto speedUpAction = std::unique_ptr<SensorAction>(
    new SpeedAction(30, 10)
  );
  
  // SwitchAction: set switch to straight position with 5ms delay
  auto switchStraightAction = std::unique_ptr<SensorAction>(
    new SwitchAction(switchId, static_cast<SwitchPosition>(SwitchPositions::STRAIGHT), 5, &trainController.getSwitchController())
  );
  
  // Create actions for when train comes from STATION_B (or anywhere else)
  // SpeedAction: set speed to 10 with 5ms delay
  auto slowDownAction = std::unique_ptr<SensorAction>(
    new SpeedAction(10, 5)
  );
  
  // SwitchAction: set switch to diverged position with 5ms delay
  auto switchDivergeAction = std::unique_ptr<SensorAction>(
    new SwitchAction(switchId, static_cast<SwitchPosition>(SwitchPositions::DIVERGED), 5, &trainController.getSwitchController())
  );
  
  // Add the conditional position action at the junction
  // This checks the train's previous position when it reaches the junction
  trainController.addConditionalPositionAction(
    junction,                    // Trigger location (where to check)
    trainIndex,                  // Which train this applies to
    stationA,                    // Condition: check if previous position was STATION_A
    std::move(speedUpAction),    // True action: if coming from STATION_A
    std::move(slowDownAction),   // False action: if coming from anywhere else
    false                        // Direction: false = forward
  );
  
  Serial.println("Conditional position actions configured:");
  Serial.println("  From STATION_A -> Speed up to 30, switch straight");
  Serial.println("  From elsewhere -> Slow to 10, switch diverged");
}

void loop() {
  // Update the train controller (this handles all automation)
  trainController.update();
  
  // Handle any manual commands from Serial (optional)
  handleSerialCommands();
}

void handleSerialCommands() {
  if (Serial.available()) {
    char command = Serial.read();
    
    switch (command) {
      case 's':
        Serial.println("Starting train...");
        trainController.setTrainSpeed(trainIndex, 20);
        break;
        
      case 'x':
        Serial.println("Stopping train...");
        trainController.setTrainSpeed(trainIndex, 0);
        break;
        
      case 'p':
        Serial.println("=== System Status ===");
        trainController.printStatus();
        break;
        
      case 'r':
        Serial.println("Reversing train direction...");
        // Get current speed and reverse it
        int currentSpeed = trainController.getTrainSpeed(trainIndex);
        trainController.setTrainSpeed(trainIndex, -currentSpeed);
        break;
        
      case 'f':
        Serial.println("Going faster...");
        trainController.setTrainSpeed(trainIndex, 50);
        break;
        
      default:
        if (command != '\n' && command != '\r') {
          Serial.println("Available commands:");
          Serial.println("  s - Start train (speed 20)");
          Serial.println("  f - Go faster (speed 50)");
          Serial.println("  x - Stop train");
          Serial.println("  r - Reverse direction");
          Serial.println("  p - Print status");
        }
        break;
    }
  }
}