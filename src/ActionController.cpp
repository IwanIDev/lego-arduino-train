#include "ActionController.hpp"
#include "Action/SequentialAction.hpp"
#include "PositionSensorController.hpp"
#include <algorithm>

ActionController::ActionController(TrainController* controller) 
    : trainController(controller), positionController(nullptr) {
}

/**
 * Execute an immediate action from a sensor.
 * @param sensor The sensor whose action should be executed
 */
void ActionController::executeAction(Sensor* sensor) {
    if (sensor != nullptr) {
        sensor->executeAction(*trainController, *this);
    }
}

/**
 * Set the position controller for position-based actions.
 * @param controller The position sensor controller to use
 */
void ActionController::setPositionController(PositionSensorController* controller) {
    positionController = controller;
}

/**
 * Add a pre-created delayed action to be managed.
 * @param action The delayed action to add
 */
void ActionController::addDelayedAction(std::unique_ptr<DelayedAction> action) {
    activeDelayedActions.push_back(std::move(action));
}

/**
 * Create and add a delayed action.
 * @param action The action to be executed after the delay
 * @param delayTimeMs The delay time in milliseconds
 */
void ActionController::addDelayedAction(std::unique_ptr<SensorAction> action, unsigned long delayTimeMs) {
    std::unique_ptr<DelayedAction> delayedAction(new DelayedAction(std::move(action), delayTimeMs));
    activeDelayedActions.push_back(std::move(delayedAction));
}

/**
 * Add a sequential action to be managed.
 * @param action The sequential action to add
 */
void ActionController::addSequentialAction(std::unique_ptr<SequentialAction> action) {
    Serial.print("ActionController::addSequentialAction() called, action is ");
    Serial.println(action ? "valid" : "null");
    Serial.print("trainController is ");
    Serial.println(trainController ? "valid" : "null");
    
    if (action && trainController != nullptr) {
        Serial.println("ActionController: Executing sequential action to initialize it");
        // Execute the sequential action to initialize it, then add it for updating
        action->execute(*trainController, *this);
        
        Serial.print("ActionController: Adding sequential action to list, current count=");
        Serial.println(activeSequentialActions.size());
        activeSequentialActions.push_back(std::move(action));
        
        Serial.print("ActionController: Sequential action added, new count=");
        Serial.println(activeSequentialActions.size());
    } else {
        Serial.println("ActionController: Cannot add sequential action - null action or trainController");
    }
}

/**
 * Update all active delayed actions. Call this regularly from your main loop.
 * This method is non-blocking and will execute actions when their delays have elapsed.
 */
void ActionController::update() {
    if (trainController == nullptr) {
        return;
    }
    
    static unsigned long lastUpdateDebug = 0;
    unsigned long currentTime = millis();
    
    // Debug output every 200ms to avoid spam
    if (currentTime - lastUpdateDebug > 200) {
        Serial.print("ActionController::update() - Active delayed: ");
        Serial.print(activeDelayedActions.size());
        Serial.print(", Active sequential: ");
        Serial.println(activeSequentialActions.size());
        lastUpdateDebug = currentTime;
    }
    
    // Update position-based actions if available
    if (positionController != nullptr) {
        positionController->checkAndExecuteActions(*trainController, *this);
    }
    
    // Update all delayed actions and remove completed ones
    activeDelayedActions.erase(
        std::remove_if(activeDelayedActions.begin(), activeDelayedActions.end(),
            [this](std::unique_ptr<DelayedAction>& action) {
                return action->update(*trainController, *this);
            }),
        activeDelayedActions.end()
    );
    
    // Update all sequential actions and remove completed ones
    Serial.print("ActionController: Before updating sequential actions, count=");
    Serial.println(activeSequentialActions.size());
    
    activeSequentialActions.erase(
        std::remove_if(activeSequentialActions.begin(), activeSequentialActions.end(),
            [this](std::unique_ptr<SequentialAction>& action) {
                bool isFinished = action->update(*trainController, *this);
                Serial.print("ActionController: SequentialAction update returned: ");
                Serial.println(isFinished ? "TRUE (remove)" : "FALSE (keep)");
                return isFinished;
            }),
        activeSequentialActions.end()
    );
    
    Serial.print("ActionController: After updating sequential actions, count=");
    Serial.println(activeSequentialActions.size());
}

/**
 * Clear all active delayed actions.
 */
void ActionController::clearAllDelayedActions() {
    activeDelayedActions.clear();
}

/**
 * Clear all active sequential actions.
 */
void ActionController::clearAllSequentialActions() {
    activeSequentialActions.clear();
}

/**
 * Clear all active actions (delayed and sequential).
 */
void ActionController::clearAllActions() {
    activeDelayedActions.clear();
    activeSequentialActions.clear();
}

/**
 * Get the number of active delayed actions.
 * @return Number of active delayed actions
 */
size_t ActionController::getActiveDelayedActionsCount() const {
    return activeDelayedActions.size();
}

/**
 * Get the number of active sequential actions.
 * @return Number of active sequential actions
 */
size_t ActionController::getActiveSequentialActionsCount() const {
    return activeSequentialActions.size();
}

/**
 * Check if there are any active delayed actions.
 * @return true if there are active delayed actions, false otherwise
 */
bool ActionController::hasActiveDelayedActions() const {
    return !activeDelayedActions.empty();
}

/**
 * Check if there are any active sequential actions.
 * @return true if there are active sequential actions, false otherwise
 */
bool ActionController::hasActiveSequentialActions() const {
    return !activeSequentialActions.empty();
}

/**
 * Check if there are any active actions (delayed or sequential).
 * @return true if there are active actions, false otherwise
 */
bool ActionController::hasActiveActions() const {
    return hasActiveDelayedActions() || hasActiveSequentialActions();
}