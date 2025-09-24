#include "ConditionalPositionAction.h"
#include "../Controllers/ActionController.h"
#include "../Position/PositionTracker.h"
#include <Arduino.h>

ConditionalPositionAction::ConditionalPositionAction(
    const SensorLocation& conditionPos,
    std::unique_ptr<SensorAction> trueAct,
    std::unique_ptr<SensorAction> falseAct,
    PositionTracker* tracker
) : conditionPosition(conditionPos),
    trueAction(std::move(trueAct)),
    falseAction(std::move(falseAct)),
    positionTracker(tracker) {
}

void ConditionalPositionAction::execute(TrainController& controller) {
    Serial.println("ConditionalPositionAction: Using deprecated execute method without ActionController");
    
    // In the simple version without ActionController, we can't access position information reliably
    // So we'll execute the true action by default as a fallback
    if (trueAction) {
        trueAction->execute(controller);
    }
}

void ConditionalPositionAction::execute(TrainController& controller, ActionController& actionController) {
    if (!positionTracker) {
        Serial.println("ConditionalPositionAction: No position tracker available, executing true action as fallback");
        if (trueAction) {
            trueAction->execute(controller, actionController);
        }
        return;
    }
    
    // Get the train's previous position
    SensorLocation previousPosition = positionTracker->getPreviousPosition();
    
    Serial.print("ConditionalPositionAction: Checking condition - Previous position: ");
    Serial.print(previousPosition.getName().c_str());
    Serial.print(", Condition position: ");
    Serial.println(conditionPosition.getName().c_str());
    
    // Check if the previous position matches our condition
    if (previousPosition == conditionPosition) {
        Serial.println("ConditionalPositionAction: Condition TRUE - executing true action");
        if (trueAction) {
            trueAction->execute(controller, actionController);
        }
    } else {
        Serial.println("ConditionalPositionAction: Condition FALSE - executing false action");
        if (falseAction) {
            falseAction->execute(controller, actionController);
        }
    }
}

std::unique_ptr<SensorAction> ConditionalPositionAction::clone() const {
    // Clone the nested actions
    std::unique_ptr<SensorAction> clonedTrueAction = trueAction ? trueAction->clone() : nullptr;
    std::unique_ptr<SensorAction> clonedFalseAction = falseAction ? falseAction->clone() : nullptr;
    
    return std::unique_ptr<SensorAction>(
        new ConditionalPositionAction(conditionPosition, std::move(clonedTrueAction), std::move(clonedFalseAction), positionTracker)
    );
}
