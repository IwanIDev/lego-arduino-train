#include <Arduino.h>
#include "DelayedAction.h"
#include "../Controllers/TrainController.h"
#include "../Controllers/ActionController.h"

/**
 * Constructor for DelayedAction.
 * @param action The action to be executed after the delay.
 * @param delayTime The delay time in milliseconds before executing the action.
 */
DelayedAction::DelayedAction(std::unique_ptr<SensorAction> action, unsigned long delayTime)
    : action(std::move(action)), delayTime(delayTime), startTime(0), isActive(false), isCompleted(false)
{

}

/**
 * Executes the delayed action on the train controller (blocking version - kept for compatibility).
 * DEPRECATED: Use update() for non-blocking execution.
 * @param controller The train controller to operate on.
 */
[[deprecated("Use update() for non-blocking execution")]]
void DelayedAction::execute(TrainController& controller) {
    // Start the timer and mark as active
    startTime = millis();
    isActive = true;
    isCompleted = false;

    Serial.println("Using deprecated execute method for DelayedAction");
    
    // Block until delay is complete, then execute
    while (!update(controller)) {
        // Small delay to prevent busy waiting
        delay(1);
    }
}

/**
 * Executes the delayed action on the train controller with ActionController (blocking version).
 * DEPRECATED: Use update() for non-blocking execution.
 * @param controller The train controller to operate on.
 * @param actionController The action controller for managing complex actions.
 */
[[deprecated("Use update() for non-blocking execution")]]
void DelayedAction::execute(TrainController& controller, ActionController& actionController) {
    // Start the timer and mark as active
    startTime = millis();
    isActive = true;
    isCompleted = false;

    Serial.println("Using deprecated execute method for DelayedAction with ActionController");
    
    // Block until delay is complete, then execute
    while (!update(controller, actionController)) {
        // Small delay to prevent busy waiting
        delay(1);
    }
}

/**
 * Non-blocking update method.
 * @param controller The train controller to operate on.
 * @return true if the action has completed, false if still waiting
 */
bool DelayedAction::update(TrainController& controller) {
    if (isCompleted) {
        return true;
    }
    
    if (!isActive) {
        // Start the timer
        startTime = millis();
        isActive = true;
        return false;
    }
    
    // Check if delay time has elapsed
    if (millis() - startTime >= delayTime) {
        // Execute the action
        if (action) {
            action->execute(controller);
        }
        isCompleted = true;
        return true;
    }
    
    return false; // Still waiting
}

/**
 * Non-blocking update method with ActionController.
 * @param controller The train controller to operate on.
 * @param actionController The action controller for managing complex actions.
 * @return true if the action has completed, false if still waiting
 */
bool DelayedAction::update(TrainController& controller, ActionController& actionController) {
    if (isCompleted) {
        Serial.println("DelayedAction: Already completed");
        return true;
    }
    
    if (!isActive) {
        // Start the timer
        startTime = millis();
        isActive = true;
        Serial.print("DelayedAction: Starting timer, delayTime=");
        Serial.print(delayTime);
        Serial.print("ms, startTime=");
        Serial.println(startTime);
        return false;
    }
    
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - startTime;
    
    // Check if delay time has elapsed
    if (elapsed >= delayTime) {
        // Execute the action with ActionController
        Serial.print("DelayedAction: Delay completed (");
        Serial.print(elapsed);
        Serial.print("ms >= ");
        Serial.print(delayTime);
        Serial.println("ms), executing action");
        
        if (action) {
            action->execute(controller, actionController);
        }
        isCompleted = true;
        return true;
    }
    
    Serial.print("DelayedAction: Still waiting, elapsed=");
    Serial.print(elapsed);
    Serial.print("ms / ");
    Serial.print(delayTime);
    Serial.println("ms");
    
    return false; // Still waiting
}

/**
 * Check if the delayed action has finished executing.
 * @return true if completed, false otherwise
 */
bool DelayedAction::isFinished() const {
    return isCompleted;
}

/**
 * Reset the delayed action to be executed again.
 */
void DelayedAction::reset() {
    isActive = false;
    isCompleted = false;
    startTime = 0;
}

/**
 * Create a fresh DelayedAction instance with the same parameters.
 * @return A new DelayedAction with the same action and delay time
 */
std::unique_ptr<DelayedAction> DelayedAction::createFresh() const {
    // Clone the inner action and create a new DelayedAction
    return std::unique_ptr<DelayedAction>(new DelayedAction(action->clone(), delayTime));
}

/**
 * Clone method for SensorAction interface
 * @return A new DelayedAction instance
 */
std::unique_ptr<SensorAction> DelayedAction::clone() const {
    return std::unique_ptr<SensorAction>(createFresh().release());
}