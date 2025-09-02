#ifndef WAITFORPOSITIONACTION_HPP
#define WAITFORPOSITIONACTION_HPP
#include "Action/SensorAction.hpp"
#include "Action/NonBlockingAction.hpp"
#include "Train/TrainInstance.hpp"

class WaitForPositionAction : public SensorAction, public NonBlockingAction {
private:
    SensorLocation targetLocation;
    TrainInstance* trainInstance;
    bool isActive;
    bool isCompleted;
    unsigned long lastLogTime;

public:
    WaitForPositionAction(TrainInstance* trainInstance, SensorLocation location);
    
    // Blocking execution methods (deprecated)
    [[deprecated("Use update() for non-blocking execution")]]
    void execute(TrainController& controller) override;
    [[deprecated("Use update() for non-blocking execution")]]
    void execute(TrainController& controller, ActionController& actionController) override;
    
    // SensorAction interface
    std::unique_ptr<SensorAction> clone() const override;
    virtual bool isWaitForPositionAction() const { return true; } // Identify as WaitForPositionAction
    bool isNonBlockingAction() const override { return true; } // Identify as NonBlockingAction
    
    // Helper method to get NonBlockingAction interface without RTTI
    NonBlockingAction* asNonBlockingAction() override { return this; }
    
    // NonBlockingAction interface implementation
    bool update(TrainController& controller, ActionController& actionController) override;
    bool isFinished() const override;
    void reset() override;
    
    // Legacy non-blocking execution method (deprecated, use NonBlockingAction interface instead)
    bool update(TrainController& controller);
    
    // State management
    // (isFinished() and reset() methods now inherited from NonBlockingAction interface)
    
    // Factory method for creating fresh instances
    std::unique_ptr<WaitForPositionAction> createFresh() const;
};

#endif // WAITFORPOSITIONACTION_HPP