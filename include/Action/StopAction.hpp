#ifndef STOP_ACTION_HPP
#define STOP_ACTION_HPP
#include "SensorAction.hpp"
#include "TrainController.hpp"

class StopAction : public SensorAction {
private:
    int delayMs;
public:
    explicit StopAction(int delay = 0);
    
    void execute(TrainController& controller) override;
};
#endif // STOP_ACTION_HPP