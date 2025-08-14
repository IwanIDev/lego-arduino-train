#include "Action/ReverseAction.hpp"
#include "ActionController.hpp"
#include "PositionTracker.hpp"
#include "PositionSensorController.hpp"
#include <memory>
#include <Arduino.h>

ReverseAction::ReverseAction(int delay)
    : delayMs(delay) {}

void ReverseAction::execute(TrainController& controller) {
    // Toggle the reverse state
    bool currentReverse = controller.getReverse();
    controller.setReverse(!currentReverse);
    
    // Don't automatically start the train if it's stopped
    // Let other actions control the speed
    
    // Optional delay after reversing
    if (delayMs > 0) {
        delay(delayMs);
    }
}

void ReverseAction::execute(TrainController& controller, ActionController& actionController) {
    // Toggle the reverse state in TrainController
    bool currentReverse = controller.getReverse();
    controller.setReverse(!currentReverse);
    
    // Also update the PositionTracker direction if available
    if (actionController.getPositionController()) {
        PositionTracker* positionTracker = actionController.getPositionController()->getPositionTracker();
        if (positionTracker) {
            // Toggle the direction in PositionTracker as well
            TrainDirection currentDirection = positionTracker->getDirection();
            TrainDirection newDirection = (currentDirection == TrainDirection::FORWARD) ? 
                                        TrainDirection::REVERSE : TrainDirection::FORWARD;
            positionTracker->setDirection(newDirection);
            
            Serial.print("ReverseAction: Updated both TrainController reverse (");
            Serial.print(!currentReverse ? "ON" : "OFF");
            Serial.print(") and PositionTracker direction (");
            Serial.print(newDirection == TrainDirection::FORWARD ? "FORWARD" : "REVERSE");
            Serial.println(")");
        }
    }
    
    // Optional delay after reversing
    if (delayMs > 0) {
        delay(delayMs);
    }
}

std::unique_ptr<SensorAction> ReverseAction::clone() const {
    return std::unique_ptr<SensorAction>(new ReverseAction(delayMs));
}
