#include "Action/StopAction.hpp"

StopAction::StopAction(int delay)
    : delayMs(delay) {}

void StopAction::execute(TrainController& controller) {
    controller.setState(STOPPED);
    if (delayMs > 0) {
        delay(delayMs);
    }
}
