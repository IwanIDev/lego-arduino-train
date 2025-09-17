#ifndef COMPOSITE_ACTION_H
#define COMPOSITE_ACTION_H

#include "SensorAction.h"
#include <vector>
#include <memory>

// Forward declaration
class ActionController;

class CompositeAction : public SensorAction {
private:
    std::vector<std::unique_ptr<SensorAction>> actions;

public:
    void addAction(std::unique_ptr<SensorAction> action);

    void execute(TrainController& controller) override;
    void execute(TrainController& controller, ActionController& actionController) override;
    std::unique_ptr<SensorAction> clone() const override;
};

#endif // COMPOSITE_ACTION_H