#ifndef DELAYED_ACTION_HPP
#define DELAYED_ACTION_HPP

#include "SensorAction.hpp"
#include <memory>

class DelayedAction : public SensorAction {
private:
    std::unique_ptr<SensorAction> action; // Pointer to the action to be executed after delay
    unsigned long delayTime;
    unsigned long startTime;
    bool isActive;
    bool isCompleted;
public:
    DelayedAction(std::unique_ptr<SensorAction> action, unsigned long delayTime);
    void execute(TrainController& controller) override;
    bool update(TrainController& controller); // Non-blocking update method
    bool isFinished() const;
    void reset();
};

#endif // DELAYED_ACTION_HPP