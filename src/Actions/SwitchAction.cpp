#include <Arduino.h>
#include "SwitchAction.h"
#include "../Controllers/TrainController.h"
#include <memory>

/**
 * Constructor for SwitchAction.
 * @param switchId The ID of the switch to control.
 * @param targetState The desired state to set the switch to (true/false).
 * @param delayMs Delay in milliseconds before executing the action.
 * @param switchController Pointer to the SwitchController instance.
 */
SwitchAction::SwitchAction(unsigned int switchId, SwitchPosition targetState, int delayMs, SwitchController* switchController)
    : switchId(switchId), targetState(targetState), delayMs(delayMs), switchController(switchController)
{
    // Constructor implementation
}

/**
 * Executes the switch action by setting the specified switch to the target state.
 * @param controller Reference to the TrainController (may be used for additional logic).
 */
void SwitchAction::execute(TrainController& controller) {
    if (switchController == nullptr) {
        Serial.println("Error: SwitchController is null in SwitchAction::execute");
        return;
    }
    
    if (!switchController->switchExists(switchId)) {
        Serial.print("Error: Switch ID ");
        Serial.print(switchId);
        Serial.println(" does not exist");
        return;
    }
    
    // Apply delay if specified
    if (delayMs > 0) {
        delay(delayMs);
    }
    
    // Set the switch to the target state
    switchController->setSwitchState(switchId, targetState);
    
    // Log the action for debugging
    Serial.print("SwitchAction executed: Switch ID ");
    Serial.print(switchId);
    Serial.print(" set to state ");
    Serial.println(targetState == SwitchPosition::STRAIGHT ? "STRAIGHT" : "DIVERGED");
}

/**
 * Creates a clone of this SwitchAction.
 * @return A unique pointer to the cloned SwitchAction.
 */
std::unique_ptr<SensorAction> SwitchAction::clone() const {
    return std::unique_ptr<SensorAction>(new SwitchAction(switchId, targetState, delayMs, switchController));
}