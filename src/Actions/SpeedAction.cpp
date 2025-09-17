#include <Arduino.h>
#include "SpeedAction.h"
#include "../Controllers/TrainController.h"
#include <memory>

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
    Serial.print("SpeedAction: Starting execution with speedChange=");
    Serial.print(speedChange);
    Serial.print(", current multiplier=");
    Serial.print(controller.getSpeedMultiplier());
    Serial.print(", current state=");
    Serial.println(controller.getState());
    
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
        Serial.println("SpeedAction: Setting state to GO");
    } else {
        controller.setState(SPEED::STOPPED);
        Serial.println("SpeedAction: Setting state to STOPPED");
    }

    Serial.print("SpeedAction: Finished execution, final multiplier=");
    Serial.print(controller.getSpeedMultiplier());
    Serial.print(", final state=");
    Serial.println(controller.getState());

    if (delayMs > 0) {
        delay(delayMs);
    }
}

std::unique_ptr<SensorAction> SpeedAction::clone() const {
    return std::unique_ptr<SensorAction>(new SpeedAction(speedChange, delayMs));
}