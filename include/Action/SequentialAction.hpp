#ifndef SEQUENTIALACTION_HPP
#define SEQUENTIALACTION_HPP

#include "Action/SensorAction.hpp"
#include <vector>
#include <memory>

// Forward declaration
class ActionController;

class SequentialAction : public SensorAction {
private:
    std::vector<std::unique_ptr<SensorAction>> actions;
public:
    SequentialAction();
    SequentialAction(std::vector<std::unique_ptr<SensorAction>>&& actionList) {
        actions = std::move(actionList);
    }
    void addAction(std::unique_ptr<SensorAction> action);
    void execute(TrainController& controller) override;
    void execute(TrainController& controller, ActionController& actionController) override;
    std::unique_ptr<SensorAction> clone() const override;
};

#endif // SEQUENTIALACTION_HPP