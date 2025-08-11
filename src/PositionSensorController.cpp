#include "PositionSensorController.hpp"
#include <Arduino.h>

PositionSensorController::PositionSensorController(PositionTracker& tracker) 
    : positionTracker(tracker) {
}

void PositionSensorController::addPositionAction(std::unique_ptr<PositionAction> action) {
    if (action) {
        Serial.print("PositionSensorController: Added position action for position ");
        Serial.println(static_cast<int>(action->getTargetPosition()));
        positionActions.push_back(std::move(action));
    }
}

void PositionSensorController::checkAndExecuteActions(TrainController& trainController, ActionController& actionController) {
    for (auto& action : positionActions) {
        if (action && !action->hasBeenExecuted() && action->checkCondition(positionTracker)) {
            Serial.print("PositionSensorController: Executing action for position ");
            Serial.println(static_cast<int>(action->getTargetPosition()));
            action->execute(trainController, actionController);
        }
    }
}

void PositionSensorController::resetAllActions() {
    Serial.println("PositionSensorController: Resetting all position actions");
    for (auto& action : positionActions) {
        if (action) {
            action->reset();
        }
    }
}

void PositionSensorController::clearActions() {
    Serial.println("PositionSensorController: Clearing all position actions");
    positionActions.clear();
}
