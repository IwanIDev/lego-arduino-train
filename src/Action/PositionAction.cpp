#include "Action/PositionAction.hpp"
#include "ActionController.hpp"
#include <Arduino.h>

// Constructor for AT_POSITION, AFTER_POSITION conditions
PositionAction::PositionAction(SensorLocation position, PositionCondition cond, std::unique_ptr<SensorAction> action)
    : targetPosition(position), secondaryPosition(position), condition(cond), 
      requiredDirection(TrainDirection::FORWARD), wrappedAction(std::move(action)), hasExecuted(false) {
}

// Constructor for BETWEEN_POSITIONS condition  
PositionAction::PositionAction(SensorLocation pos1, SensorLocation pos2, std::unique_ptr<SensorAction> action)
    : targetPosition(pos1), secondaryPosition(pos2), condition(PositionCondition::BETWEEN_POSITIONS),
      requiredDirection(TrainDirection::FORWARD), wrappedAction(std::move(action)), hasExecuted(false) {
}

// Constructor for DIRECTION_AT_POSITION condition
PositionAction::PositionAction(SensorLocation position, TrainDirection direction, std::unique_ptr<SensorAction> action)
    : targetPosition(position), secondaryPosition(position), condition(PositionCondition::DIRECTION_AT_POSITION),
      requiredDirection(direction), wrappedAction(std::move(action)), hasExecuted(false) {
}

void PositionAction::execute(TrainController& controller) {
    if (hasExecuted) {
        Serial.println("PositionAction: Already executed, skipping");
        return;
    }
    
    if (wrappedAction) {
        Serial.print("PositionAction: Executing wrapped action at position ");
        Serial.println(static_cast<int>(targetPosition));
        wrappedAction->execute(controller);
        hasExecuted = true;
    } else {
        Serial.println("PositionAction: No wrapped action to execute");
    }
}

void PositionAction::execute(TrainController& controller, ActionController& actionController) {
    if (hasExecuted) {
        Serial.println("PositionAction: Already executed, skipping");
        return;
    }
    
    if (wrappedAction) {
        Serial.print("PositionAction: Executing wrapped action at position ");
        Serial.println(static_cast<int>(targetPosition));
        wrappedAction->execute(controller, actionController);
        hasExecuted = true;
    } else {
        Serial.println("PositionAction: No wrapped action to execute");
    }
}

std::unique_ptr<SensorAction> PositionAction::clone() const {
    std::unique_ptr<SensorAction> clonedWrapped = nullptr;
    if (wrappedAction) {
        clonedWrapped = wrappedAction->clone();
    }
    
    switch (condition) {
        case PositionCondition::AT_POSITION:
        case PositionCondition::AFTER_POSITION:
            return std::unique_ptr<SensorAction>(new PositionAction(targetPosition, condition, std::move(clonedWrapped)));
            
        case PositionCondition::BETWEEN_POSITIONS:
            return std::unique_ptr<SensorAction>(new PositionAction(targetPosition, secondaryPosition, std::move(clonedWrapped)));
            
        case PositionCondition::DIRECTION_AT_POSITION:
            return std::unique_ptr<SensorAction>(new PositionAction(targetPosition, requiredDirection, std::move(clonedWrapped)));
    }
    
    return nullptr;
}

bool PositionAction::checkCondition(const PositionTracker& positionTracker) const {
    SensorLocation currentPos = positionTracker.getCurrentPosition();
    SensorLocation previousPos = positionTracker.getPreviousPosition();
    TrainDirection currentDir = positionTracker.getDirection();
    
    switch (condition) {
        case PositionCondition::AT_POSITION:
            return currentPos == targetPosition;
            
        case PositionCondition::AFTER_POSITION:
            return previousPos == targetPosition && currentPos != targetPosition;
            
        case PositionCondition::BETWEEN_POSITIONS:
            // Check if train moved from one position to the other
            return (previousPos == targetPosition && currentPos == secondaryPosition) ||
                   (previousPos == secondaryPosition && currentPos == targetPosition);
                   
        case PositionCondition::DIRECTION_AT_POSITION:
            return currentPos == targetPosition && currentDir == requiredDirection;
    }
    
    return false;
}

void PositionAction::reset() {
    hasExecuted = false;
    Serial.println("PositionAction: Reset execution state");
}
