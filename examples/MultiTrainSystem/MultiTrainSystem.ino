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

// Create custom sensor locations for the track layout
SensorLocation westStation("WEST_STATION", 1);
SensorLocation westTunnel("WEST_TUNNEL", 2);
SensorLocation eastStation("EAST_STATION", 3);
SensorLocation eastTunnel("EAST_TUNNEL", 4);

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
  
  // Add trains to the system using custom sensor locations
  train1Index = trainController.addTrain(TRAIN1_HUB_NAME, MOTOR_PORT, westStation);
  train2Index = trainController.addTrain(TRAIN2_HUB_NAME, MOTOR_PORT, eastStation);
  
  Serial.print("Added Train 1 with index: ");
  Serial.println(train1Index);
  Serial.print("Added Train 2 with index: ");
  Serial.println(train2Index);
  
  // Add reed switch sensors using custom sensor locations
  trainController.addReedSwitchSensor(WEST_STATION_SENSOR, westStation);
  trainController.addReedSwitchSensor(WEST_TUNNEL_SENSOR, westTunnel);
  trainController.addReedSwitchSensor(EAST_STATION_SENSOR, eastStation);
  trainController.addReedSwitchSensor(EAST_TUNNEL_SENSOR, eastTunnel);
  
  // Example: Add custom sensor locations for more complex layouts
  // SensorLocation bridgeEntry("BRIDGE_ENTRANCE", 100);
  // SensorLocation junctionPoint("MAIN_JUNCTION", 101);  
  // trainController.addReedSwitchSensor(D13, bridgeEntry);
  // trainController.addReedSwitchSensor(D14, junctionPoint);
  
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
  trainController.addTrackSegment(westStation, 
                                 westTunnel, 
                                 westTunnel, 
                                 train1Index);
  
  trainController.addTrackSegment(westTunnel, 
                                 eastTunnel, 
                                 westStation, 
                                 train1Index);
  
  trainController.addTrackSegment(eastTunnel, 
                                 eastStation, 
                                 westTunnel, 
                                 train1Index);
  
  trainController.addTrackSegment(eastStation, 
                                 eastTunnel, 
                                 eastTunnel, 
                                 train1Index);
}

void setupTrackLayoutTrain2() {
  // Define track segments for Train 2 (opposite direction)
  trainController.addTrackSegment(eastStation, 
                                 eastTunnel, 
                                 eastTunnel, 
                                 train2Index);
  
  trainController.addTrackSegment(eastTunnel, 
                                 westTunnel, 
                                 eastStation, 
                                 train2Index);
  
  trainController.addTrackSegment(westTunnel, 
                                 westStation, 
                                 eastTunnel, 
                                 train2Index);
  
  trainController.addTrackSegment(westStation, 
                                 westTunnel, 
                                 westTunnel, 
                                 train2Index);
}

void setupAutomatedActions() {
  // Train 1 actions: Stop at stations, slow through tunnels
  trainController.addStopAction(westStation, train1Index, 0);
  trainController.addStopAction(eastStation, train1Index, 0);
  
  // Create sequential action for tunnel entry (Train 1)
  std::vector<ActionConfig> tunnelActions1;
  tunnelActions1.push_back(ActionConfig(TrainActionType::SPEED)); // Slow down
  tunnelActions1.back().speed = 0;
  tunnelActions1.back().targetSpeed = 20;
  
  tunnelActions1.push_back(ActionConfig(TrainActionType::SWITCH)); // Set switches
  tunnelActions1.back().switchId = switch1Id;
  tunnelActions1.back().switchPosition = SwitchPositions::DIVERGED;
  
  trainController.addSequentialAction(westTunnel, train1Index, tunnelActions1);
  
  // Train 2 actions: Different behavior pattern
  std::vector<ActionConfig> stationActions2;
  stationActions2.push_back(ActionConfig(TrainActionType::STOP));
  stationActions2.back().speed = 0;
  
  // Add delay before restart
  ActionConfig delayedRestart(TrainActionType::DELAY);
  delayedRestart.delayMs = 2000;
  delayedRestart.delayedAction = std::unique_ptr<ActionConfig>(new ActionConfig(TrainActionType::SPEED));
  delayedRestart.delayedAction->targetSpeed = 25;
  stationActions2.push_back(std::move(delayedRestart));
  
  trainController.addSequentialAction(westStation, train2Index, stationActions2);
  trainController.addSequentialAction(eastStation, train2Index, stationActions2);
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

/*
 * CUSTOM SENSOR LOCATION EXAMPLES
 * 
 * Here are examples of how to create and use custom sensor locations
 * for more complex track layouts:
 * 
 * // Create custom locations with descriptive names and unique IDs
 * SensorLocation mainJunction("MAIN_JUNCTION", 100);
 * SensorLocation bridgeEntrance("BRIDGE_ENTRANCE", 101);  
 * SensorLocation tunnelExit("TUNNEL_EXIT", 102);
 * SensorLocation maintenanceYard("MAINTENANCE_YARD", 200);
 * SensorLocation loadingDock("LOADING_DOCK", 201);
 * 
 * // Add sensors at these custom locations
 * trainController.addReedSwitchSensor(D13, mainJunction);
 * trainController.addLightSensor(A2, 30, bridgeEntrance);
 * trainController.addReedSwitchSensor(D14, tunnelExit);
 * 
 * // Use custom locations in track layout
 * trainController.addTrackSegment(mainJunction, bridgeEntrance, tunnelExit, train1Index);
 * trainController.addTrackSegment(tunnelExit, maintenanceYard, loadingDock, train2Index);
 * 
 * // Add train starting at custom location
 * size_t maintenanceTrain = trainController.addTrain("MaintenanceTrain", 
 *                                                   PoweredUpHubPort::B, 
 *                                                   maintenanceYard);
 * 
 * // Create automated actions for custom locations
 * std::vector<ActionConfig> maintenanceActions;
 * maintenanceActions.push_back(ActionConfig(TrainActionType::STOP));
 * maintenanceActions.push_back(ActionConfig(TrainActionType::REVERSE));
 * trainController.addSequentialAction(loadingDock, maintenanceTrain, maintenanceActions);
 * 
 * This flexibility allows you to create complex layouts with meaningful
 * location names and organize your track system however makes sense
 * for your specific LEGO train setup.
 */