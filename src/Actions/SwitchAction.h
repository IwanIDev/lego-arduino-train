#ifndef SWITCH_ACTION_H
#define SWITCH_ACTION_H

#include "SensorAction.h"
#include "../Switch/SwitchController.h"
#include <memory>

class SwitchAction : public SensorAction {
private:
    int switchId; // ID of the switch to control
    SwitchPosition targetState; // Target state to set the switch to
    int delayMs; // Delay in milliseconds before executing the action
    SwitchController* switchController; // Pointer to the switch controller

public:
    SwitchAction(unsigned int switchId, SwitchPosition targetState, int delayMs, SwitchController* switchController);
    void execute(TrainController& controller) override;
    std::unique_ptr<SensorAction> clone() const override;
};

#endif // SWITCH_ACTION_H