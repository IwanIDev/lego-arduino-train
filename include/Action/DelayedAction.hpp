#ifndef DELAYED_ACTION_HPP
#define DELAYED_ACTION_HPP

#include "SensorAction.hpp"
#include <memory>

class DelayedAction : public SensorAction {
private:
    std::unique_ptr<SensorAction> action; // Pointer to the action to be executed after delay
    unsigned long delayTime;
public:
    DelayedAction(std::unique_ptr<SensorAction> action, unsigned long delayTime);
    void execute(TrainController& controller) override;
};

#endif // DELAYED_ACTION_HPP