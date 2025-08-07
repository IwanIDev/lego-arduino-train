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
    std::unique_ptr<SensorAction> clone() const override;
    bool isDelayedAction() const override { return true; } // Identify as DelayedAction
    bool update(TrainController& controller); // Non-blocking update method
    bool isFinished() const;
    void reset();
    
    // Method to create a new DelayedAction instance with the same parameters
    std::unique_ptr<DelayedAction> createFresh() const;
};

#endif // DELAYED_ACTION_HPP