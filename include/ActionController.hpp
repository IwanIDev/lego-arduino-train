#ifndef ACTION_CONTROLLER_HPP
#define ACTION_CONTROLLER_HPP

#include "TrainController.hpp"
#include "Sensor.hpp"
#include "Action/DelayedAction.hpp"
#include "Action/SensorAction.hpp"
#include <vector>
#include <memory>

// Forward declarations
class SequentialAction;
class PositionSensorController;

class ActionController {
private:
    TrainController* trainController;
    std::vector<std::unique_ptr<DelayedAction>> activeDelayedActions;
    std::vector<std::unique_ptr<SequentialAction>> activeSequentialActions;
    PositionSensorController* positionController; // Optional position-based controller

public:
    explicit ActionController(TrainController* controller);
    
    // Original sensor-based action execution
    void executeAction(Sensor* sensor);
    
    // Position-based action management
    void setPositionController(PositionSensorController* controller);
    PositionSensorController* getPositionController() const { return positionController; }
    
    // Delayed action management
    void addDelayedAction(std::unique_ptr<DelayedAction> action);
    void addDelayedAction(std::unique_ptr<SensorAction> action, unsigned long delayTimeMs);
    
    // Sequential action management
    void addSequentialAction(std::unique_ptr<SequentialAction> action);
    
    // Non-blocking update method - call this from your main loop
    void update();
    
    // Management methods
    void clearAllDelayedActions();
    void clearAllSequentialActions();
    void clearAllActions();
    size_t getActiveDelayedActionsCount() const;
    size_t getActiveSequentialActionsCount() const;
    bool hasActiveDelayedActions() const;
    bool hasActiveSequentialActions() const;
    bool hasActiveActions() const;
};
#endif // ACTION_CONTROLLER_HPP