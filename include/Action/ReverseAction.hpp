#ifndef REVERSE_ACTION_HPP
#define REVERSE_ACTION_HPP
#include "SensorAction.hpp"
#include "TrainController.hpp"

class ReverseAction : public SensorAction {
private:
    int delayMs;
public:
    explicit ReverseAction(int delay = 0);
    
    void execute(TrainController& controller) override;
};
#endif // REVERSE_ACTION_HPP
