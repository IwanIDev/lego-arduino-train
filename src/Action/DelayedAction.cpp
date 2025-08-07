#include <Arduino.h>
#include "Action/DelayedAction.hpp"
#include "TrainController.hpp"

/**
 * Constructor for DelayedAction.
 * @param action The action to be executed after the delay.
 * @param delayTime The delay time in milliseconds before executing the action.
 */
DelayedAction::DelayedAction(std::unique_ptr<SensorAction> action, unsigned long delayTime)
    : action(std::move(action)), delayTime(delayTime)
{

}

/**
 * Executes the delayed action on the train controller.
 * @param controller The train controller to operate on.
 */
void DelayedAction::execute(TrainController& controller) {
    // 1. Delay for the specified time
    delay(delayTime);

    // 2. Execute the action after the delay
    if (action) {
        action->execute(controller);
    }
}