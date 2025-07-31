#pragma once
#include "SensorAction.hpp"
#include "TrainController.hpp"

class StopAction : public SensorAction {
private:
    int delayMs;
public:
    explicit StopAction(int delay = 0) : delayMs(delay) {}
    
    void execute(TrainController& controller) override {
        controller.setState(STOPPED);
        if (delayMs > 0) {
            delay(delayMs);
        }
    }
};