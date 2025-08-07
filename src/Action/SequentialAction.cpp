#include "Action/SequentialAction.hpp"
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
    for (auto& action : actions) {
        if (action) {
            action->execute(controller);
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