#ifndef WAITFORPOSITIONACTION_HPP
#define WAITFORPOSITIONACTION_HPP
#include "Action/SensorAction.hpp"
#include "Train/TrainInstance.hpp"

class WaitForPositionAction : public SensorAction {
private:
    SensorLocation targetLocation;
    TrainInstance* trainInstance;
public:
    WaitForPositionAction(TrainInstance* trainInstance, SensorLocation location);
    void execute(TrainController& controller, ActionController& actionController) override;
};

#endif // WAITFORPOSITIONACTION_HPP