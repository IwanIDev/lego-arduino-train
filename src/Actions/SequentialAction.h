#ifndef SEQUENTIAL_ACTION_H
#define SEQUENTIAL_ACTION_H

#include "SensorAction.h"
#include "NonBlockingAction.h"
#include "DelayedAction.h"
#include <vector>
#include <memory>

// Forward declaration
class ActionController;

class SequentialAction : public SensorAction, public NonBlockingAction {
private:
    std::vector<std::unique_ptr<SensorAction>> actions;
    
    // State management for non-blocking execution
    size_t currentActionIndex;
    NonBlockingAction* currentNonBlockingAction; // Changed from DelayedAction to NonBlockingAction
    bool isExecuting;
    
public:
    SequentialAction();
    SequentialAction(std::vector<std::unique_ptr<SensorAction>>&& actionList) {
        actions = std::move(actionList);
        currentActionIndex = 0;
        currentNonBlockingAction = nullptr;
        isExecuting = false;
    }
    void addAction(std::unique_ptr<SensorAction> action);
    void execute(TrainController& controller) override;
    void execute(TrainController& controller, ActionController& actionController) override;
    std::unique_ptr<SensorAction> clone() const override;
    bool isSequentialAction() const override { return true; }
    bool isNonBlockingAction() const override { return true; }
    
    // Helper method to get NonBlockingAction interface without RTTI
    NonBlockingAction* asNonBlockingAction() override { return this; }
    
    // NonBlockingAction interface implementation
    bool update(TrainController& controller, ActionController& actionController) override;
    bool isFinished() const override;
    void reset() override;
    
    // Method to create a new SequentialAction instance with the same parameters
    std::unique_ptr<SequentialAction> createFresh() const;
    
    size_t getActionCount() const { return actions.size(); }
    size_t getCurrentActionIndex() const { return currentActionIndex; }
};

#endif // SEQUENTIAL_ACTION_H