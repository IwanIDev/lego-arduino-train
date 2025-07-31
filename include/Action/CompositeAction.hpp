#ifndef COMPOSITEACTION_HPP
#define COMPOSITEACTION_HPP

#include "Action/SensorAction.hpp"
#include <vector>
#include <memory>

class CompositeAction : public SensorAction {
private:
    std::vector<std::unique_ptr<SensorAction>> actions;

public:
    void addAction(std::unique_ptr<SensorAction> action);

    void execute(TrainController& controller) override;
};

#endif // COMPOSITEACTION_HPP