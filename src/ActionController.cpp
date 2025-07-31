#include "ActionController.hpp"

ActionController::ActionController(TrainController* controller) 
    : trainController(controller) {
}

void ActionController::executeAction(LightSensor* sensor) {
    if (sensor != nullptr) {
        sensor->executeAction(*trainController);
    }
}