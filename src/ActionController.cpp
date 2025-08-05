#include "ActionController.hpp"

ActionController::ActionController(TrainController* controller) 
    : trainController(controller) {
}

void ActionController::executeAction(Sensor* sensor) {
    if (sensor != nullptr) {
        sensor->executeAction(*trainController);
    }
}