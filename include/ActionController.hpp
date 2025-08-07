#ifndef ACTION_CONTROLLER_HPP
#define ACTION_CONTROLLER_HPP

#include "TrainController.hpp"
#include "Sensor.hpp"
#include "Action/DelayedAction.hpp"
#include "Action/SensorAction.hpp"
#include <vector>
#include <memory>

class ActionController {
private:
    TrainController* trainController;
    std::vector<std::unique_ptr<DelayedAction>> activeDelayedActions;

public:
    explicit ActionController(TrainController* controller);
    
    // Original sensor-based action execution
    void executeAction(Sensor* sensor);
    
    // Delayed action management
    void addDelayedAction(std::unique_ptr<DelayedAction> action);
    void addDelayedAction(std::unique_ptr<SensorAction> action, unsigned long delayTimeMs);
    
    // Non-blocking update method - call this from your main loop
    void update();
    
    // Management methods
    void clearAllDelayedActions();
    size_t getActiveDelayedActionsCount() const;
    bool hasActiveDelayedActions() const;
};
#endif // ACTION_CONTROLLER_HPP