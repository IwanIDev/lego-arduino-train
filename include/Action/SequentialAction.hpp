#ifndef SEQUENTIALACTION_HPP
#define SEQUENTIALACTION_HPP

#include "Action/SensorAction.hpp"
#include "Action/DelayedAction.hpp"
#include <vector>
#include <memory>

// Forward declaration
class ActionController;

class SequentialAction : public SensorAction {
private:
    std::vector<std::unique_ptr<SensorAction>> actions;
    
    // State management for non-blocking execution
    size_t currentActionIndex;
    std::unique_ptr<DelayedAction> currentDelayedAction;
    bool isExecuting;
    
public:
    SequentialAction();
    SequentialAction(std::vector<std::unique_ptr<SensorAction>>&& actionList) {
        actions = std::move(actionList);
        currentActionIndex = 0;
        isExecuting = false;
    }
    void addAction(std::unique_ptr<SensorAction> action);
    void execute(TrainController& controller) override;
    void execute(TrainController& controller, ActionController& actionController) override;
    std::unique_ptr<SensorAction> clone() const override;
    bool isSequentialAction() const override { return true; } // Identify as SequentialAction
    
    // Non-blocking execution methods
    bool update(TrainController& controller, ActionController& actionController);
    void reset();
    bool isFinished() const;
    bool isActive() const { return isExecuting; }

    std::unique_ptr<SequentialAction> createFresh() const;
};

#endif // SEQUENTIALACTION_HPP