#ifndef START_ACTION_H
#define START_ACTION_H

#include "SensorAction.h"
#include <memory>

class StartAction : public SensorAction {
private:
    float speed;
    bool forceReverse = false; // Added to track reverse state
    int delayMs;
public:
    StartAction(float speed, int delayMs, bool forceReverse = false);
    void execute(TrainController& controller) override;
    std::unique_ptr<SensorAction> clone() const override;
};

#endif // START_ACTION_H