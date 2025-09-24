/**
 * ConditionalPositionAction Usage Example
 * 
 * This example demonstrates how to use the ConditionalPositionAction to handle
 * complex routes where the same sensor is triggered from different paths.
 * 
 * Scenario: 
 * - Train passes over sensor "JUNCTION" from two different paths
 * - If coming from "STATION_A", switch to track 1 and speed up
 * - If coming from "STATION_B", switch to track 2 and slow down
 */

#include "LegoTrainController.h"

void setupConditionalPositionExample() {
    LegoTrainController trainController;
    
    // Initialize the system
    trainController.begin();
    
    // Add a train
    SensorLocation startPosition("START", 1);
    size_t trainIndex = trainController.addTrain("MyTrain", 0, startPosition);
    
    // Add sensors
    trainController.addLightSensor(A0, 50, SensorLocation("STATION_A", 2));
    trainController.addLightSensor(A1, 50, SensorLocation("STATION_B", 3));
    trainController.addLightSensor(A2, 50, SensorLocation("JUNCTION", 4));
    
    // Add a switch
    int switchId = trainController.addSwitch(7, SwitchPositions::STRAIGHT);
    
    // Define positions
    SensorLocation stationA("STATION_A", 2);
    SensorLocation stationB("STATION_B", 3);
    SensorLocation junction("JUNCTION", 4);
    
    // Create action configurations for when coming from STATION_A
    ActionConfig speedUpAction(TrainActionType::SPEED);
    speedUpAction.targetSpeed = 30;
    speedUpAction.speed = 10;
    
    ActionConfig switchToTrack1(TrainActionType::SWITCH);
    switchToTrack1.switchId = switchId;
    switchToTrack1.switchPosition = SwitchPositions::STRAIGHT;
    switchToTrack1.speed = 5;
    
    // Create action configurations for when coming from STATION_B  
    ActionConfig slowDownAction(TrainActionType::SPEED);
    slowDownAction.targetSpeed = 10;
    slowDownAction.speed = 5;
    
    ActionConfig switchToTrack2(TrainActionType::SWITCH);
    switchToTrack2.switchId = switchId;
    switchToTrack2.switchPosition = SwitchPositions::DIVERGED;
    switchToTrack2.speed = 5;
    
    // Add the conditional position action at the junction
    // This will check the train's previous position when it reaches the junction
    trainController.addConditionalPositionAction(
        junction,           // Trigger location
        trainIndex,         // Train index
        stationA,          // Condition: check if previous position was STATION_A
        speedUpAction,      // True action: speed up if coming from STATION_A
        slowDownAction,     // False action: slow down if coming from elsewhere
        false              // Forward direction
    );
    
    // You can also create more complex conditional actions using SequentialAction
    // For example, a sequence that both changes switch position AND speed:
    
    std::vector<ActionConfig> fromStationASequence;
    fromStationASequence.push_back(switchToTrack1);  // First switch tracks
    fromStationASequence.push_back(speedUpAction);   // Then speed up
    
    std::vector<ActionConfig> fromStationBSequence;
    fromStationBSequence.push_back(switchToTrack2);  // First switch tracks
    fromStationBSequence.push_back(slowDownAction);  // Then slow down
    
    // Use the sequential action method directly with the vector of actions
    trainController.addSequentialAction(junction, trainIndex, fromStationASequence, false);
    
    // You can also nest conditional actions within sequential actions
    // for even more complex behavior patterns
}

/**
 * Alternative example using ActionConfig with CONDITIONAL_POSITION type
 * This shows how to configure conditional actions using the ActionConfig struct
 */
void setupConditionalPositionWithActionConfig() {
    LegoTrainController trainController;
    trainController.begin();
    
    // Setup train and sensors (same as above example)
    SensorLocation startPosition("START", 1);
    size_t trainIndex = trainController.addTrain("MyTrain", 0, startPosition);
    
    // Define positions
    SensorLocation stationA("STATION_A", 2);
    SensorLocation junction("JUNCTION", 4);
    
    // Create a conditional action configuration
    ActionConfig conditionalConfig(TrainActionType::CONDITIONAL_POSITION);
    conditionalConfig.conditionPosition = stationA;  // Check if coming from STATION_A
    
    // Define true action (speed up if from STATION_A)
    conditionalConfig.trueAction = std::unique_ptr<ActionConfig>(new ActionConfig(TrainActionType::SPEED));
    conditionalConfig.trueAction->targetSpeed = 25;
    conditionalConfig.trueAction->speed = 10;
    
    // Define false action (slow down if from elsewhere)
    conditionalConfig.falseAction = std::unique_ptr<ActionConfig>(new ActionConfig(TrainActionType::SPEED));
    conditionalConfig.falseAction->targetSpeed = 15;
    conditionalConfig.falseAction->speed = 5;
    
    // This could be used in a sequential action or other complex action pattern
    std::vector<ActionConfig> complexSequence;
    complexSequence.push_back(std::move(conditionalConfig));
    
    // Add as sequential action
    trainController.addSequentialAction(junction, trainIndex, complexSequence, false);
}