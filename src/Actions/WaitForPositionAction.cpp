#include <Arduino.h>
#include "WaitForPositionAction.h"
#include "../Controllers/TrainController.h"
#include "../Controllers/ActionController.h"
#include "../Position/PositionTracker.h"

/**
 * Constructor for WaitForPositionAction.
 * @param trainInstance The train instance to monitor for position changes.
 * @param location The target location to wait for.
 */
WaitForPositionAction::WaitForPositionAction(TrainInstance* trainInstance, SensorLocation location)
    : trainInstance(trainInstance), targetLocation(location), isActive(false), isCompleted(false), lastLogTime(0)
{
    lastLogTime = 0;
}

/**
 * Executes the wait action on the train controller (blocking version - kept for compatibility).
 * DEPRECATED: Use update() for non-blocking execution.
 * @param controller The train controller to operate on.
 */
[[deprecated("Use update() for non-blocking execution")]]
void WaitForPositionAction::execute(TrainController& controller) {
    Serial.println("WaitForPositionAction: Using deprecated execute method");
    
    // Mark as active
    isActive = true;
    isCompleted = false;
    
    // Block until train reaches target position
    // Note: In blocking mode, we don't have an ActionController, so pass nullptr
    while (!updateInternal(controller)) {
        // Small delay to prevent busy waiting
        delay(10);
    }
}

/**
 * Executes the wait action on the train controller with ActionController (blocking version).
 * DEPRECATED: Use update() for non-blocking execution.
 * @param controller The train controller to operate on.
 * @param actionController The action controller for managing complex actions.
 */
[[deprecated("Use update() for non-blocking execution")]]
void WaitForPositionAction::execute(TrainController& controller, ActionController& actionController) {
    Serial.println("WaitForPositionAction: Using deprecated execute method with ActionController");
    
    // Mark as active
    isActive = true;
    isCompleted = false;
    
    // Block until train reaches target position
    while (!update(controller, actionController)) {
        // Small delay to prevent busy waiting
        delay(10);
    }
}

/**
 * Internal update method (helper for both blocking and non-blocking modes).
 * @param controller The train controller to operate on.
 * @return true if the train has reached the target position, false if still waiting
 */
bool WaitForPositionAction::updateInternal(TrainController& controller) {
    if (isCompleted) {
        return true;
    }
    
    if (!isActive) {
        // Mark as active and start monitoring
        isActive = true;
        Serial.print("WaitForPositionAction: Starting to wait for position ");
        Serial.println(static_cast<int>(targetLocation));
        return false;
    }
    
    // Check if we have a valid train instance and position tracker
    if (!trainInstance) {
        Serial.println("WaitForPositionAction: Error - No train instance provided");
        isCompleted = true;
        return true;
    }
    
    PositionTracker* positionTracker = trainInstance->getPositionTracker();
    if (!positionTracker) {
        Serial.println("WaitForPositionAction: Error - No position tracker available");
        isCompleted = true;
        return true;
    }
    
    // Check current position
    SensorLocation currentPosition = positionTracker->getCurrentPosition();
    
    if (currentPosition == targetLocation) {
        Serial.print("WaitForPositionAction: Train reached target position ");
        Serial.println(static_cast<int>(targetLocation));
        isCompleted = true;
        return true;
    }
    
    // Still waiting - optionally log progress
    unsigned long currentTime = millis();
    if (currentTime - lastLogTime > 5000) { // Log every 5 seconds
        Serial.print("WaitForPositionAction: Still waiting for position ");
        Serial.print(static_cast<int>(targetLocation));
        Serial.print(", current position: ");
        Serial.println(static_cast<int>(currentPosition));
        lastLogTime = currentTime;
    }
    
    return false; // Still waiting
}

/**
 * Non-blocking update method with ActionController.
 * @param controller The train controller to operate on.
 * @param actionController The action controller for managing complex actions.
 * @return true if the train has reached the target position, false if still waiting
 */
bool WaitForPositionAction::update(TrainController& controller, ActionController& actionController) {
    // For this action, the ActionController doesn't change the behavior
    // We delegate to the internal update method
    return updateInternal(controller);
}

/**
 * Check if the wait action has finished (train reached target position).
 * @return true if completed, false otherwise
 */
bool WaitForPositionAction::isFinished() const {
    return isCompleted;
}

/**
 * Reset the wait action to be executed again.
 */
void WaitForPositionAction::reset() {
    isActive = false;
    isCompleted = false;
    Serial.print("WaitForPositionAction: Reset to wait for position ");
    Serial.println(static_cast<int>(targetLocation));
}

/**
 * Create a fresh WaitForPositionAction instance with the same parameters.
 * @return A new WaitForPositionAction with the same train instance and target location
 */
std::unique_ptr<WaitForPositionAction> WaitForPositionAction::createFresh() const {
    return std::unique_ptr<WaitForPositionAction>(new WaitForPositionAction(trainInstance, targetLocation));
}

/**
 * Clone method for SensorAction interface
 * @return A new WaitForPositionAction instance
 */
std::unique_ptr<SensorAction> WaitForPositionAction::clone() const {
    return std::unique_ptr<SensorAction>(createFresh().release());
}
