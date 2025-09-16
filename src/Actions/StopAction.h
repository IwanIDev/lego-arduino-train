#ifndef STOP_ACTION_H
#define STOP_ACTION_H
#include "SensorAction.h"
#include "../Controllers/TrainController.h"
#include <memory>

class StopAction : public SensorAction {
private:
    int delayMs;
public:
    explicit StopAction(int delay = 0);
    
    void execute(TrainController& controller) override;
    std::unique_ptr<SensorAction> clone() const override;
};
#endif // STOP_ACTION_H