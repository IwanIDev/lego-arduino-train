#ifndef STARTACTION_HPP
#define STARTACTION_HPP

#include "SensorAction.hpp"

class StartAction : public SensorAction {
private:
    float speed;
    bool forceReverse = false; // Added to track reverse state
    int delayMs;
public:
    StartAction(float speed, int delayMs, bool forceReverse = false);
    void execute(TrainController& controller) override;
};

#endif // STARTACTION_HPP