#ifndef REVERSE_ACTION_H
#define REVERSE_ACTION_H
#include "SensorAction.h"
#include "../Controllers/TrainController.h"
#include <memory>

// Forward declarations
class ActionController;

class ReverseAction : public SensorAction {
private:
    int delayMs;
public:
    explicit ReverseAction(int delay = 0);
    
    void execute(TrainController& controller) override;
    void execute(TrainController& controller, ActionController& actionController) override;
    std::unique_ptr<SensorAction> clone() const override;
};
#endif // REVERSE_ACTION_H