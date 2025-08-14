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
    if (action && trainController != nullptr) {
        // Execute the sequential action to initialize it, then add it for updating
        action->execute(*trainController, *this);
        activeSequentialActions.push_back(std::move(action));
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
    activeSequentialActions.erase(
        std::remove_if(activeSequentialActions.begin(), activeSequentialActions.end(),
            [this](std::unique_ptr<SequentialAction>& action) {
                return action->update(*trainController, *this);
            }),
        activeSequentialActions.end()
    );
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