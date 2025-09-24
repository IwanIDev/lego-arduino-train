#ifndef CONDITIONAL_POSITION_ACTION_H
#define CONDITIONAL_POSITION_ACTION_H

#include "SensorAction.h"
#include "../Position/SensorLocation.h"
#include "../Position/PositionTracker.h"

// Forward declarations
class ActionController;

class ConditionalPositionAction : public SensorAction {
private:
    SensorLocation conditionPosition;
    std::unique_ptr<SensorAction> trueAction;
    std::unique_ptr<SensorAction> falseAction;
    PositionTracker* positionTracker; // Pointer to position tracker for checking previous position

public:
    ConditionalPositionAction(
        const SensorLocation& conditionPos,
        std::unique_ptr<SensorAction> trueAct,
        std::unique_ptr<SensorAction> falseAct,
        PositionTracker* tracker
    );

    void execute(TrainController& controller) override;
    void execute(TrainController& controller, ActionController& actionController) override;
    std::unique_ptr<SensorAction> clone() const override;
};

#endif // CONDITIONAL_POSITION_ACTION_H