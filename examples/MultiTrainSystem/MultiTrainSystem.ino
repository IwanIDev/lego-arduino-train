/*
 * MultiTrainSystem - Advanced LEGO Train Controller Example
 * 
 * This example demonstrates advanced features including:
 * - Multiple trains
 * - Automated track layout with sensors
 * - Switch control
 * - Sequential actions
 * - Position tracking
 * 
 * Hardware required:
 * - Arduino ESP32 or compatible
 * - Multiple LEGO Powered Up trains with hubs
 * - Reed switch sensors at track positions
 * - Relay modules for switch control
 * 
 * Created by IwanIDev
 */

#include <LegoTrainController.h>

// Create the main controller instance
LegoTrainController trainController;

// Train configurations
const String TRAIN1_HUB_NAME = "Train1";
const String TRAIN2_HUB_NAME = "Train2";
const byte MOTOR_PORT = (byte)PoweredUpHubPort::B;

// Sensor pins
const int WEST_STATION_SENSOR = D9;
const int WEST_TUNNEL_SENSOR = D10;
const int EAST_STATION_SENSOR = D11;
const int EAST_TUNNEL_SENSOR = D12;

// Switch control pins
const int SWITCH_1_PIN = D5;
const int SWITCH_2_PIN = D6;

// Train indices
size_t train1Index;
size_t train2Index;

// Switch IDs
int switch1Id;
int switch2Id;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("=== LEGO Train Controller - Multi-Train System ===");
  
  // Initialize the library
  if (!trainController.begin()) {
    Serial.println("Failed to initialize train controller!");
    return;
  }
  
  // Add trains to the system
  train1Index = trainController.addTrain(TRAIN1_HUB_NAME, MOTOR_PORT, SensorLocations::WEST_STATION);
  train2Index = trainController.addTrain(TRAIN2_HUB_NAME, MOTOR_PORT, SensorLocations::EAST_STATION);
  
  Serial.print("Added Train 1 with index: ");
  Serial.println(train1Index);
  Serial.print("Added Train 2 with index: ");
  Serial.println(train2Index);
  
  // Add reed switch sensors
  trainController.addReedSwitchSensor(WEST_STATION_SENSOR, SensorLocations::WEST_STATION);
  trainController.addReedSwitchSensor(WEST_TUNNEL_SENSOR, SensorLocations::WEST_TUNNEL);
  trainController.addReedSwitchSensor(EAST_STATION_SENSOR, SensorLocations::EAST_STATION);
  trainController.addReedSwitchSensor(EAST_TUNNEL_SENSOR, SensorLocations::EAST_TUNNEL);
  
  // Add track switches
  switch1Id = trainController.addSwitch(SWITCH_1_PIN, SwitchPositions::STRAIGHT);
  switch2Id = trainController.addSwitch(SWITCH_2_PIN, SwitchPositions::STRAIGHT);
  
  // Setup track layout for Train 1
  setupTrackLayoutTrain1();
  
  // Setup track layout for Train 2  
  setupTrackLayoutTrain2();
  
  // Setup automated actions
  setupAutomatedActions();
  
  // Enable debug output
  trainController.enableDebug(true);
  
  Serial.println("Multi-train system setup complete!");
  Serial.println("Trains will operate automatically based on sensor detection.");
  Serial.println("Send 'p' for status, 's' to start trains, 'x' to stop all");
  Serial.println();
}

void setupTrackLayoutTrain1() {
  // Define track segments for Train 1
  trainController.addTrackSegment(SensorLocations::WEST_STATION, 
                                 SensorLocations::WEST_TUNNEL, 
                                 SensorLocations::WEST_TUNNEL, 
                                 train1Index);
  
  trainController.addTrackSegment(SensorLocations::WEST_TUNNEL, 
                                 SensorLocations::EAST_TUNNEL, 
                                 SensorLocations::WEST_STATION, 
                                 train1Index);
  
  trainController.addTrackSegment(SensorLocations::EAST_TUNNEL, 
                                 SensorLocations::EAST_STATION, 
                                 SensorLocations::WEST_TUNNEL, 
                                 train1Index);
  
  trainController.addTrackSegment(SensorLocations::EAST_STATION, 
                                 SensorLocations::EAST_TUNNEL, 
                                 SensorLocations::EAST_TUNNEL, 
                                 train1Index);
}

void setupTrackLayoutTrain2() {
  // Define track segments for Train 2 (opposite direction)
  trainController.addTrackSegment(SensorLocations::EAST_STATION, 
                                 SensorLocations::EAST_TUNNEL, 
                                 SensorLocations::EAST_TUNNEL, 
                                 train2Index);
  
  trainController.addTrackSegment(SensorLocations::EAST_TUNNEL, 
                                 SensorLocations::WEST_TUNNEL, 
                                 SensorLocations::EAST_STATION, 
                                 train2Index);
  
  trainController.addTrackSegment(SensorLocations::WEST_TUNNEL, 
                                 SensorLocations::WEST_STATION, 
                                 SensorLocations::EAST_TUNNEL, 
                                 train2Index);
  
  trainController.addTrackSegment(SensorLocations::WEST_STATION, 
                                 SensorLocations::WEST_TUNNEL, 
                                 SensorLocations::WEST_TUNNEL, 
                                 train2Index);
}

void setupAutomatedActions() {
  // Train 1 actions: Stop at stations, slow through tunnels
  trainController.addStopAction(SensorLocations::WEST_STATION, train1Index, 0);
  trainController.addStopAction(SensorLocations::EAST_STATION, train1Index, 0);
  
  // Create sequential action for tunnel entry (Train 1)
  std::vector<ActionConfig> tunnelActions1;
  tunnelActions1.push_back(ActionConfig(ActionType::SPEED)); // Slow down
  tunnelActions1.back().speed = 0;
  tunnelActions1.back().targetSpeed = 20;
  
  tunnelActions1.push_back(ActionConfig(ActionType::SWITCH)); // Set switches
  tunnelActions1.back().switchId = switch1Id;
  tunnelActions1.back().switchPosition = SwitchPositions::DIVERGED;
  
  trainController.addSequentialAction(SensorLocations::WEST_TUNNEL, train1Index, tunnelActions1);
  
  // Train 2 actions: Different behavior pattern
  std::vector<ActionConfig> stationActions2;
  stationActions2.push_back(ActionConfig(ActionType::STOP));
  stationActions2.back().speed = 0;
  
  // Add delay before restart
  ActionConfig delayedRestart(ActionType::DELAY);
  delayedRestart.delayMs = 2000;
  delayedRestart.delayedAction = std::make_unique<ActionConfig>(ActionType::SPEED);
  delayedRestart.delayedAction->targetSpeed = 25;
  stationActions2.push_back(std::move(delayedRestart));
  
  trainController.addSequentialAction(SensorLocations::WEST_STATION, train2Index, stationActions2);
  trainController.addSequentialAction(SensorLocations::EAST_STATION, train2Index, stationActions2);
}

void loop() {
  // Update the train controller (handles all automation)
  trainController.update();
  
  // Handle serial commands
  handleSerialCommands();
  
  delay(10);
}

void handleSerialCommands() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    
    switch (command) {
      case 's':
        Serial.println("Starting automated train operations");
        trainController.setTrainSpeed(train1Index, 30);
        trainController.setTrainSpeed(train2Index, 25);
        break;
        
      case 'x':
        Serial.println("Stopping all trains");
        trainController.stopTrain(train1Index);
        trainController.stopTrain(train2Index);
        break;
        
      case 'p':
        trainController.printStatus();
        break;
        
      case '1':
        Serial.println("Operating switch 1");
        trainController.operateSwitch(switch1Id, SwitchPositions::DIVERGED);
        break;
        
      case '2':
        Serial.println("Operating switch 2");
        trainController.operateSwitch(switch2Id, SwitchPositions::STRAIGHT);
        break;
        
      default:
        break;
    }
  }
}