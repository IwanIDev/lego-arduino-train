#include "Action/SequentialAction.hpp"
#include "Action/DelayedAction.hpp"
#include "Action/WaitForPositionAction.hpp"
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
        Serial.println("SequentialAction: update() - not executing, returning true");
        return true; // Already finished
    }
    
    Serial.print("SequentialAction: update() called, currentActionIndex=");
    Serial.print(currentActionIndex);
    Serial.print(", total actions=");
    Serial.println(actions.size());
        
    while (currentActionIndex < actions.size()) {
        auto& action = actions[currentActionIndex];
        
        if (!action) {
            Serial.println("Encountered a null action in SequentialAction, skipping.");
            currentActionIndex++;
            continue;
        }

        if (!action->isDelayedAction() && !action->isWaitForPositionAction()) {
            // Execute immediate action and move to next
            Serial.print("SequentialAction: Executing immediate action #");
            Serial.println(currentActionIndex);
            action->execute(controller, actionController);
            currentActionIndex++;
            continue;
        }

        if (action->isDelayedAction()) {
            if (!currentDelayedAction) {
                // Start the delayed action
                Serial.print("SequentialAction: Starting delayed action #");
                Serial.println(currentActionIndex);
                DelayedAction* delayedAction = static_cast<DelayedAction*>(action.get());
                currentDelayedAction = delayedAction->createFresh();
            }

            // Update the delayed action
            Serial.print("SequentialAction: Updating delayed action #");
            Serial.println(currentActionIndex);
            if (!currentDelayedAction->update(controller, actionController)) {
                // Still waiting for delayed action to complete
                Serial.println("SequentialAction: Delayed action still running, waiting...");
                Serial.println("SequentialAction: update() returning FALSE (still active)");
                return false;
            }
            
            // Delayed action completed, move to next
            Serial.print("SequentialAction: Delayed action #");
            Serial.print(currentActionIndex);
            Serial.println(" completed, moving to next");
            currentDelayedAction.reset();
            currentActionIndex++;
            continue;
        }

        if (action->isWaitForPositionAction()) {
            // Handle WaitForPositionAction with non-blocking update
            Serial.print("SequentialAction: Updating WaitForPositionAction #");
            Serial.println(currentActionIndex);
            
            // Try to cast to WaitForPositionAction to access update method
            // Note: This requires including the WaitForPositionAction header
            WaitForPositionAction* waitAction = static_cast<WaitForPositionAction*>(action.get());
            if (!waitAction->update(controller, actionController)) {
                // Still waiting for position to be reached
                Serial.println("SequentialAction: WaitForPositionAction still waiting, returning false");
                return false;
            }
            
            // Position reached, move to next action
            Serial.print("SequentialAction: WaitForPositionAction #");
            Serial.print(currentActionIndex);
            Serial.println(" completed, moving to next");
            currentActionIndex++;
            continue;
        }

    }
    // All actions completed
    Serial.println("SequentialAction: All actions completed");
    Serial.println("SequentialAction: update() returning TRUE (finished)");
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