#ifndef SPEEDACTION_HPP
#define SPEEDACTION_HPP

#include "SensorAction.hpp"

class SpeedAction : public SensorAction {
private:
    int speedChange; // Speed change value
    int delayMs; // Delay in milliseconds before executing the action
public:
    SpeedAction(int speedChange, int delayMs);
    void execute(TrainController& controller) override;
};

#endif // SPEEDACTION_HPP