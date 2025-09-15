#ifndef SWITCHACTION_HPP
#define SWITCHACTION_HPP

#include "SensorAction.hpp"
#include "Switch/SwitchController.hpp"
#include <memory>

class SwitchAction : public SensorAction {
private:
    int switchId; // ID of the switch to control
    bool targetState; // Target state to set the switch to
    int delayMs; // Delay in milliseconds before executing the action
    SwitchController* switchController; // Pointer to the switch controller

public:
    SwitchAction(int switchId, bool targetState, int delayMs, SwitchController* switchController);
    void execute(TrainController& controller) override;
    std::unique_ptr<SensorAction> clone() const override;
};

#endif // SWITCHACTION_HPP