#ifndef ACTION_CONTROLLER_HPP
#define ACTION_CONTROLLER_HPP

#include <map>
#include "TrainController.hpp"
#include "Action/SensorAction.hpp"

class ActionController {
private:
    TrainController* trainController;
    std::map<SensorLocation, SensorAction> actionMap;

public:
    ActionController(TrainController* controller);

    void handleSensorTrigger(SensorLocation location);
};
#endif // ACTION_CONTROLLER_HPP