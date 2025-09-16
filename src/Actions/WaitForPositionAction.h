#ifndef WAIT_FOR_POSITION_ACTION_H
#define WAIT_FOR_POSITION_ACTION_H
#include "SensorAction.h"
#include "NonBlockingAction.h"
#include "../Train/TrainInstance.h"

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
    
    // Additional accessors
    SensorLocation getTargetLocation() const { return targetLocation; }
    TrainInstance* getTrainInstance() const { return trainInstance; }
};

#endif // WAIT_FOR_POSITION_ACTION_H