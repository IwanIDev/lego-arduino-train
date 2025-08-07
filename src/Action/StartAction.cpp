#include <Arduino.h>
#include "Action/StartAction.hpp"
#include "TrainController.hpp"
#include <memory>

/**
 * Constructor for StartAction.
 * @param speed The speed to set the train to.
 * @param delayMs Delay in milliseconds before starting the action.
 * @param forceReverse If true, the train will start in reverse.
 */
StartAction::StartAction(float speed, int delayMs, bool forceReverse)
    : speed(speed), delayMs(delayMs), forceReverse(forceReverse) 
{

}

/**
 * Executes the start action on the train controller.
 * @param controller The train controller to operate on.
 */
void StartAction::execute(TrainController& controller) {
    // 1. Set reverse state if forceReverse is true
    if (forceReverse != controller.getReverse()) {
        controller.setReverse(forceReverse);
    }

    // 2. Set desired speed of the train
    float currentSpeedMultiplier = controller.getSpeedMultiplier();
    float desiredSpeedMultiplier = speed;

    // 3. Gradually adjust speed multiplier
    while (currentSpeedMultiplier < desiredSpeedMultiplier) {
        controller.incrementSpeed();
        currentSpeedMultiplier = controller.getSpeedMultiplier();
        if (currentSpeedMultiplier >= desiredSpeedMultiplier) {
            break; // Exit loop if desired speed is reached
        }
    }

    while (currentSpeedMultiplier > desiredSpeedMultiplier) {
        controller.decrementSpeed();
        currentSpeedMultiplier = controller.getSpeedMultiplier();
        if (currentSpeedMultiplier <= desiredSpeedMultiplier) {
            break; // Exit loop if desired speed is reached
        }
    }

    // 4. Set train state to GO
    if (speed > 0) {
        controller.setState(SPEED::GO);
    }

    if (delayMs > 0) {
        // 5. Delay for the specified time
        delay(delayMs);
    }
}

std::unique_ptr<SensorAction> StartAction::clone() const {
    return std::unique_ptr<SensorAction>(new StartAction(speed, delayMs, forceReverse));
}
