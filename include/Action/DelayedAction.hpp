#ifndef DELAYED_ACTION_HPP
#define DELAYED_ACTION_HPP

#include "SensorAction.hpp"
#include "NonBlockingAction.hpp"
#include <memory>

class DelayedAction : public SensorAction, public NonBlockingAction {
private:
    std::unique_ptr<SensorAction> action; // Pointer to the action to be executed after delay
    unsigned long delayTime;
    unsigned long startTime;
    bool isActive;
    bool isCompleted;
public:
    DelayedAction(std::unique_ptr<SensorAction> action, unsigned long delayTime);
    void execute(TrainController& controller) override;
    void execute(TrainController& controller, ActionController& actionController) override;
    std::unique_ptr<SensorAction> clone() const override;
    bool isDelayedAction() const override { return true; } // Identify as DelayedAction
    bool isNonBlockingAction() const override { return true; } // Identify as NonBlockingAction
    
    // NonBlockingAction interface implementation
    bool update(TrainController& controller, ActionController& actionController) override; // Non-blocking update with ActionController
    bool isFinished() const override;
    void reset() override;
    
    // Legacy non-blocking update method (deprecated, use NonBlockingAction interface instead)
    bool update(TrainController& controller); // Non-blocking update method
    
    // Method to create a new DelayedAction instance with the same parameters
    std::unique_ptr<DelayedAction> createFresh() const;
};

#endif // DELAYED_ACTION_HPP