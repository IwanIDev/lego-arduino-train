#include "Action/StopAction.hpp"
#include <memory>

StopAction::StopAction(int delay)
    : delayMs(delay) {}

void StopAction::execute(TrainController& controller) {
    controller.setState(STOPPED);
    if (delayMs > 0) {
        delay(delayMs);
    }
}

std::unique_ptr<SensorAction> StopAction::clone() const {
    return std::unique_ptr<SensorAction>(new StopAction(delayMs));
}
