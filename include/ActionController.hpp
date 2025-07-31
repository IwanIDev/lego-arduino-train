#ifndef ACTION_CONTROLLER_HPP
#define ACTION_CONTROLLER_HPP

#include <map>
#include "TrainController.hpp"
#include "SensorAction.hpp"

class ActionController {
private:
    TrainController* trainController;
    std::map<SensorLocation, SensorAction> actionMap;

public:
    ActionController(TrainController* controller) : trainController(controller) {
        // Example actions
        actionMap[SensorLocation::STATION_STOP] = {SPEED::STOPPED, false, 5000};
        actionMap[SensorLocation::SPEED_REDUCE] = {SPEED::GO, false, 0};
        actionMap[SensorLocation::DIRECTION_CHANGE] = {SPEED::GO, true, 1000};
    }

    void handleSensorTrigger(SensorLocation location) {
        if (actionMap.count(location) == 0) return;
        
        auto& action = actionMap[location];
        trainController->setState(action.targetSpeed);
        
        if (action.changeDirection) {
            trainController->setReverse(!trainController->getReverse());
        }
        
        if (action.delayMs > 0) {
            delay(action.delayMs);
        }
    }
};
#endif // ACTION_CONTROLLER_HPP