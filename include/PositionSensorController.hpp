#ifndef POSITIONSENSORCONTROLLER_HPP
#define POSITIONSENSORCONTROLLER_HPP

#include "PositionTracker.hpp"
#include "Action/PositionAction.hpp"
#include "ActionController.hpp"
#include "TrainController.hpp"
#include <vector>
#include <memory>

class PositionSensorController {
private:
    PositionTracker& positionTracker;
    std::vector<std::unique_ptr<PositionAction>> positionActions;
    
public:
    PositionSensorController(PositionTracker& tracker);
    
    // Add position-based actions
    void addPositionAction(std::unique_ptr<PositionAction> action);
    
    // Check all position actions and execute if conditions are met
    void checkAndExecuteActions(TrainController& trainController, ActionController& actionController);
    
    // Access to position tracker
    PositionTracker* getPositionTracker() { return &positionTracker; }
    
    // Reset all position actions (useful when starting a new loop)
    void resetAllActions();
    
    // Remove all actions
    void clearActions();
    
    // Get number of registered actions
    size_t getActionCount() const { return positionActions.size(); }
};

#endif // POSITIONSENSORCONTROLLER_HPP
