#include "ActionController.hpp"
#include <algorithm>

ActionController::ActionController(TrainController* controller) 
    : trainController(controller) {
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
 * Update all active delayed actions. Call this regularly from your main loop.
 * This method is non-blocking and will execute actions when their delays have elapsed.
 */
void ActionController::update() {
    if (trainController == nullptr) {
        return;
    }
    
    // Update all actions and remove completed ones
    activeDelayedActions.erase(
        std::remove_if(activeDelayedActions.begin(), activeDelayedActions.end(),
            [this](std::unique_ptr<DelayedAction>& action) {
                return action->update(*trainController);
            }),
        activeDelayedActions.end()
    );
}

/**
 * Clear all active delayed actions.
 */
void ActionController::clearAllDelayedActions() {
    activeDelayedActions.clear();
}

/**
 * Get the number of active delayed actions.
 * @return Number of active delayed actions
 */
size_t ActionController::getActiveDelayedActionsCount() const {
    return activeDelayedActions.size();
}

/**
 * Check if there are any active delayed actions.
 * @return true if there are active delayed actions, false otherwise
 */
bool ActionController::hasActiveDelayedActions() const {
    return !activeDelayedActions.empty();
}