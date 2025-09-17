#ifndef SPEED_ACTION_H
#define SPEED_ACTION_H

#include "SensorAction.h"
#include <memory>

class SpeedAction : public SensorAction {
private:
    int speedChange; // Speed change value
    int delayMs; // Delay in milliseconds before executing the action
public:
    SpeedAction(int speedChange, int delayMs);
    void execute(TrainController& controller) override;
    std::unique_ptr<SensorAction> clone() const override;
};

#endif // SPEED_ACTION_H