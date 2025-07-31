#include "Action/CompositeAction.hpp"

void CompositeAction::addAction(std::unique_ptr<SensorAction> action) {
    actions.push_back(std::move(action));
}

void CompositeAction::execute(TrainController& controller) {
    for (auto& action : actions) {
        action->execute(controller);
    }
}