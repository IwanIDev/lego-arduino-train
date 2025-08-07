#ifndef SPEEDACTION_HPP
#define SPEEDACTION_HPP

#include "SensorAction.hpp"
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

#endif // SPEEDACTION_HPP