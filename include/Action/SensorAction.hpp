#pragma once
#include "TrainController.hpp"

class SensorAction {
public:
    virtual ~SensorAction() = default;
    virtual void execute(TrainController& controller) = 0;
};