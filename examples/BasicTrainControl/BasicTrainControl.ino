/*
 * BasicTrainControl - Simple LEGO Train Controller Example
 * 
 * This example demonstrates basic train control using the LEGO Train Controller library.
 * It shows how to:
 * - Initialize the library
 * - Add a train
 * - Add sensors
 * - Control train speed and direction
 * 
 * Hardware required:
 * - Arduino ESP32 or compatible
 * - LEGO Powered Up train with hub
 * - Reed switch sensors (optional)
 * 
 * Created by IwanIDev
 */

#include <LegoTrainController.h>

// Create the main controller instance
LegoTrainController trainController;

// Train configuration
const String TRAIN_HUB_NAME = "Train1";
const byte MOTOR_PORT = (byte)PoweredUpHubPort::B;

// Sensor pins (optional)
const int SENSOR_PIN_1 = D9;  // Reed switch at station 1
const int SENSOR_PIN_2 = D10; // Reed switch at station 2

// Variables for manual control
unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 100; // Update every 100ms

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("=== LEGO Train Controller - Basic Example ===");
  
  // Initialize the library
  if (!trainController.begin()) {
    Serial.println("Failed to initialize train controller!");
    return;
  }
  
  // Add a train to the system
  size_t trainIndex = trainController.addTrain(TRAIN_HUB_NAME, MOTOR_PORT, SensorLocations::WEST_STATION);
  Serial.print("Added train with index: ");
  Serial.println(trainIndex);
  
  // Add reed switch sensors (optional)
  trainController.addReedSwitchSensor(SENSOR_PIN_1, SensorLocations::WEST_STATION);
  trainController.addReedSwitchSensor(SENSOR_PIN_2, SensorLocations::EAST_STATION);
  
  // Enable debug output
  trainController.enableDebug(true);
  
  Serial.println("Setup complete! Train controller ready.");
  Serial.println("Send commands via Serial:");
  Serial.println("  's' - Start train (speed 30)");
  Serial.println("  'f' - Go faster (speed 50)");
  Serial.println("  'x' - Stop train");
  Serial.println("  'r' - Reverse direction");
  Serial.println("  'p' - Print status");
  Serial.println();
}

void loop() {
  // Update the train controller (this handles all automation)
  trainController.update();
  
  // Handle serial commands for manual control
  handleSerialCommands();
  
  // Periodic status updates
  if (millis() - lastUpdate > UPDATE_INTERVAL) {
    // You can add periodic checks here
    lastUpdate = millis();
  }
  
  delay(10); // Small delay to prevent overwhelming the system
}

void handleSerialCommands() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    
    switch (command) {
      case 's':
        Serial.println("Starting train at speed 30");
        trainController.setTrainSpeed(0, 30);
        break;
        
      case 'f':
        Serial.println("Increasing speed to 50");
        trainController.setTrainSpeed(0, 50);
        break;
        
      case 'x':
        Serial.println("Stopping train");
        trainController.stopTrain(0);
        break;
        
      case 'r':
        Serial.println("Reversing train direction");
        trainController.reverseTrain(0);
        break;
        
      case 'p':
        trainController.printStatus();
        break;
        
      default:
        // Ignore unknown commands
        break;
    }
  }
}