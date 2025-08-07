#include "Action/CompositeAction.hpp"
#include "Action/DelayedAction.hpp"
#include "ActionController.hpp"
#include <Arduino.h>

void CompositeAction::addAction(std::unique_ptr<SensorAction> action) {
    actions.push_back(std::move(action));
}

void CompositeAction::execute(TrainController& controller) {
    Serial.println("Warning: Using deprecated execute method for CompositeAction without ActionController");
    for (auto& action : actions) {
        if (action->isDelayedAction()) {
            Serial.println("Error: DelayedAction found in CompositeAction but no ActionController available");
            Serial.println("DelayedAction will use deprecated blocking execute method");
        }
        action->execute(controller);
    }
}

void CompositeAction::execute(TrainController& controller, ActionController& actionController) {
    for (auto& action : actions) {
        if (action->isDelayedAction()) {
            // For DelayedAction, create a fresh instance and add it to ActionController
            DelayedAction* delayedAction = static_cast<DelayedAction*>(action.get());
            actionController.addDelayedAction(delayedAction->createFresh());
            Serial.println("Added DelayedAction to ActionController for non-blocking execution");
        } else {
            // For immediate actions, execute directly
            action->execute(controller);
        }
    }
}

std::unique_ptr<SensorAction> CompositeAction::clone() const {
    std::unique_ptr<CompositeAction> cloned(new CompositeAction());
    for (const auto& action : actions) {
        cloned->addAction(action->clone());
    }
    return std::unique_ptr<SensorAction>(cloned.release());
}