#include "Action/CompositeAction.hpp"

void CompositeAction::addAction(std::unique_ptr<SensorAction> action) {
    actions.push_back(std::move(action));
}

void CompositeAction::execute(TrainController& controller) {
    for (auto& action : actions) {
        action->execute(controller);
    }
}

std::unique_ptr<SensorAction> CompositeAction::clone() const {
    std::unique_ptr<CompositeAction> cloned(new CompositeAction());
    for (const auto& action : actions) {
        cloned->addAction(action->clone());
    }
    return std::unique_ptr<SensorAction>(cloned.release());
}