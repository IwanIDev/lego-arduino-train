#include <Arduino.h>
#include "Action/SpeedAction.hpp"
#include "TrainController.hpp"

/**
 * Constructor for SpeedAction.
 * @param speedChange The amount to change the speed by (positive for increase, negative for decrease).
 * @param delayMs Delay in milliseconds before executing the action.
 */
SpeedAction::SpeedAction(int speedChange, int delayMs)
    : speedChange(speedChange), delayMs(delayMs)
{

}

/**
 * Executes the speed change action on the train controller.
 * @param controller The train controller to operate on.
 */
void SpeedAction::execute(TrainController& controller) {
    // 1. Apply speed change
    for (int i = 0; i < abs(speedChange); ++i) {
        if (speedChange > 0) {
            controller.incrementSpeed();
        } else {
            controller.decrementSpeed();
        }
    }

    // 2. Set train state based on speed change
    if (controller.getSpeedMultiplier() > 0) {
        controller.setState(SPEED::GO);
    } else {
        controller.setState(SPEED::STOPPED);
    }

    if (delayMs > 0) {
        delay(delayMs);
    }
}