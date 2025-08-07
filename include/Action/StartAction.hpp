#ifndef STARTACTION_HPP
#define STARTACTION_HPP

#include "SensorAction.hpp"
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

#endif // STARTACTION_HPP

#endif // STARTACTION_HPP