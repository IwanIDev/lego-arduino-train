#include "Action/ReverseAction.hpp"
#include <memory>

ReverseAction::ReverseAction(int delay)
    : delayMs(delay) {}

void ReverseAction::execute(TrainController& controller) {
    // Toggle the reverse state
    bool currentReverse = controller.getReverse();
    controller.setReverse(!currentReverse);
    
    // Don't automatically start the train if it's stopped
    // Let other actions control the speed
    
    // Optional delay after reversing
    if (delayMs > 0) {
        delay(delayMs);
    }
}

std::unique_ptr<SensorAction> ReverseAction::clone() const {
    return std::unique_ptr<SensorAction>(new ReverseAction(delayMs));
}
