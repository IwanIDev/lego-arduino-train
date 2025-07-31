#ifndef ACTION_CONTROLLER_HPP
#define ACTION_CONTROLLER_HPP

#include "TrainController.hpp"
#include "LightSensor.hpp"

class ActionController {
private:
    TrainController* trainController;

public:
    explicit ActionController(TrainController* controller);
    void executeAction(LightSensor* sensor);
};
#endif // ACTION_CONTROLLER_HPP