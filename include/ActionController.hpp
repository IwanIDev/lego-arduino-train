#ifndef ACTION_CONTROLLER_HPP
#define ACTION_CONTROLLER_HPP

#include "TrainController.hpp"
#include "Sensor.hpp"

class ActionController {
private:
    TrainController* trainController;

public:
    explicit ActionController(TrainController* controller);
    void executeAction(Sensor* sensor);
};
#endif // ACTION_CONTROLLER_HPP