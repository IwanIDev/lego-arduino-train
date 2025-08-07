#include "Action/SequentialAction.hpp"
#include "Action/DelayedAction.hpp"
#include "ActionController.hpp"
#include <Arduino.h>
#include "TrainController.hpp"

/**
 * Constructor for SequentialAction.
 */
SequentialAction::SequentialAction()  
{
    actions.reserve(10);  // Reserve space for 10 actions
}

/**
 * Adds a new action to the sequence.
 * @param action The action to be added.
 */
void SequentialAction::addAction(std::unique_ptr<SensorAction> action) {
    if (action) {
        actions.push_back(std::move(action));
    } else {
        Serial.println("Attempted to add a null action to SequentialAction.");
    }
}

/**
 * Executes all actions in the sequence.
 * @param controller The train controller to operate on.
 */
void SequentialAction::execute(TrainController& controller) {
    Serial.println("Warning: Using deprecated execute method for SequentialAction without ActionController");
    for (auto& action : actions) {
        if (action) {
            if (action->isDelayedAction()) {
                Serial.println("Error: DelayedAction found in SequentialAction but no ActionController available");
                Serial.println("DelayedAction will use deprecated blocking execute method");
            }
            action->execute(controller);
        } else {
            Serial.println("Encountered a null action in SequentialAction.");
        }
    }
}

/**
 * Executes all actions in the sequence with proper DelayedAction handling.
 * @param controller The train controller to operate on.
 * @param actionController The action controller for managing delayed actions.
 */
void SequentialAction::execute(TrainController& controller, ActionController& actionController) {
    for (auto& action : actions) {
        if (action) {
            if (action->isDelayedAction()) {
                // For DelayedAction, create a fresh instance and add it to ActionController
                DelayedAction* delayedAction = static_cast<DelayedAction*>(action.get());
                actionController.addDelayedAction(delayedAction->createFresh());
                Serial.println("Added DelayedAction to ActionController for non-blocking execution");
            } else {
                // For immediate actions, execute directly
                action->execute(controller);
            }
        } else {
            Serial.println("Encountered a null action in SequentialAction.");
        }
    }
}

/**
 * Clones the SequentialAction, creating a new instance with the same actions.
 * @return A unique pointer to the cloned SequentialAction.
 */
std::unique_ptr<SensorAction> SequentialAction::clone() const {
    std::unique_ptr<SequentialAction> cloned(new SequentialAction());
    for (const auto& action : actions) {
        if (action) {
            cloned->addAction(action->clone());
        } else {
            Serial.println("Encountered a null action while cloning SequentialAction.");
        }
    }
    return std::unique_ptr<SensorAction>(cloned.release());
}