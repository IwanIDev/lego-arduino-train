#include "Action/SequentialAction.hpp"
#include "Action/DelayedAction.hpp"
#include "ActionController.hpp"
#include <Arduino.h>
#include "TrainController.hpp"

/**
 * Constructor for SequentialAction.
 */
SequentialAction::SequentialAction() : currentActionIndex(0), isExecuting(false)
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
 * This version starts the sequential execution and relies on update() for completion.
 * @param controller The train controller to operate on.
 * @param actionController The action controller for managing delayed actions.
 */
void SequentialAction::execute(TrainController& controller, ActionController& actionController) {
    if (!isExecuting) {
        Serial.println("Starting SequentialAction execution");
        isExecuting = true;
        currentActionIndex = 0;
        currentDelayedAction.reset();
    }
    
    // Start the first update cycle - the ActionController will need to call update() 
    // repeatedly until this SequentialAction is finished
    update(controller, actionController);
}

/**
 * Clones the SequentialAction, creating a new instance with the same actions.
 * @return A unique pointer to the cloned SequentialAction.
 */
std::unique_ptr<SensorAction> SequentialAction::clone() const {
    return std::unique_ptr<SensorAction>(createFresh().release());
}

/**
 * Non-blocking update method for SequentialAction.
 * This method processes actions one at a time, waiting for DelayedActions to complete
 * before proceeding to the next action in the sequence.
 * @param controller The train controller to operate on.
 * @param actionController The action controller for managing delayed actions.
 * @return true if all actions in the sequence have completed, false if still executing.
 */
bool SequentialAction::update(TrainController& controller, ActionController& actionController) {
    if (!isExecuting) {
        return true; // Already finished
    }
        
    while (currentActionIndex < actions.size()) {
        auto& action = actions[currentActionIndex];
        
        if (!action) {
            Serial.println("Encountered a null action in SequentialAction, skipping.");
            currentActionIndex++;
            continue;
        }

        if (!action->isDelayedAction()) {
            // Execute immediate action and move to next
            action->execute(controller);
            currentActionIndex++;
            continue;
        }

        if (!currentDelayedAction) {
            // Start the delayed action
            DelayedAction* delayedAction = static_cast<DelayedAction*>(action.get());
            currentDelayedAction = delayedAction->createFresh();
        }

        // Update the delayed action
        if (!currentDelayedAction->update(controller)) {
            // Still waiting for delayed action to complete
            return false;
        }
        
        // Delayed action completed, move to next
        currentDelayedAction.reset();
        currentActionIndex++;

    }
    // All actions completed
    isExecuting = false;
    currentActionIndex = 0;
    return true;
}

/**
 * Reset the SequentialAction to be executed again.
 */
void SequentialAction::reset() {
    isExecuting = false;
    currentActionIndex = 0;
    currentDelayedAction.reset();
}

/**
 * Check if the SequentialAction has finished executing.
 * @return true if all actions have completed, false if still executing.
 */
bool SequentialAction::isFinished() const {
    return !isExecuting;
}

std::unique_ptr<SequentialAction> SequentialAction::createFresh() const {
    std::unique_ptr<SequentialAction> fresh(new SequentialAction());
    for (const auto& action : actions) {
        if (action) {
            fresh->addAction(action->clone());
        } else {
            Serial.println("Encountered a null action while creating fresh SequentialAction.");
        }
    }
    return fresh;
}