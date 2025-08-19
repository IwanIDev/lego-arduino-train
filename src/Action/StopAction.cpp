#include "Action/StopAction.hpp"
#include <memory>

StopAction::StopAction(int delay)
    : delayMs(delay) {}

void StopAction::execute(TrainController& controller) {
    Serial.print("StopAction: Executing with delayMs=");
    Serial.print(delayMs);
    Serial.print(", current state=");
    Serial.println(controller.getState());
    
    controller.setState(STOPPED);
    
    Serial.print("StopAction: State set to STOPPED, final state=");
    Serial.println(controller.getState());
    
    if (delayMs > 0) {
        delay(delayMs);
    }
}

std::unique_ptr<SensorAction> StopAction::clone() const {
    return std::unique_ptr<SensorAction>(new StopAction(delayMs));
}
