#ifndef WAITFORPOSITIONACTION_HPP
#define WAITFORPOSITIONACTION_HPP
#include "Action/SensorAction.hpp"
#include "Train/TrainInstance.hpp"

class WaitForPositionAction : public SensorAction {
private:
    SensorLocation targetLocation;
    TrainInstance* trainInstance;
    bool isActive;
    bool isCompleted;

public:
    WaitForPositionAction(TrainInstance* trainInstance, SensorLocation location);
    
    // Blocking execution methods (deprecated)
    [[deprecated("Use update() for non-blocking execution")]]
    void execute(TrainController& controller) override;
    [[deprecated("Use update() for non-blocking execution")]]
    void execute(TrainController& controller, ActionController& actionController) override;
    
    // Non-blocking execution methods
    bool update(TrainController& controller);
    bool update(TrainController& controller, ActionController& actionController);
    
    // SensorAction interface
    std::unique_ptr<SensorAction> clone() const override;
    virtual bool isWaitForPositionAction() const { return true; } // Identify as WaitForPositionAction
    
    // State management
    bool isFinished() const;
    void reset();
    
    // Factory method for creating fresh instances
    std::unique_ptr<WaitForPositionAction> createFresh() const;
};

#endif // WAITFORPOSITIONACTION_HPP