#include "Action/ReverseAction.hpp"
#include <memory>

ReverseAction::ReverseAction(int delay)
    : delayMs(delay) {}

void ReverseAction::execute(TrainController& controller) {
    // Toggle the reverse state
    bool currentReverse = controller.getReverse();
    controller.setReverse(!currentReverse);
    
    // If the train is currently stopped (speedMultiplier = 0), give it some initial speed
    // so the reverse action has an effect
    if (controller.getSpeedMultiplier() <= 0) {
        controller.incrementSpeed(); // Give it some initial speed
        controller.setState(GO);      // Make sure it's set to GO state
    }
    
    // Optional delay after reversing
    if (delayMs > 0) {
        delay(delayMs);
    }
}

std::unique_ptr<SensorAction> ReverseAction::clone() const {
    return std::unique_ptr<SensorAction>(new ReverseAction(delayMs));
}
