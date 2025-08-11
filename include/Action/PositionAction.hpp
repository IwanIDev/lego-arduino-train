#ifndef POSITIONACTION_HPP
#define POSITIONACTION_HPP

#include "SensorAction.hpp"
#include "PositionTracker.hpp"
#include <memory>
#include <functional>

enum class PositionCondition {
    AT_POSITION,      // Trigger when train is at a specific position
    BETWEEN_POSITIONS, // Trigger when train is between two positions
    AFTER_POSITION,   // Trigger when train has passed a specific position
    DIRECTION_AT_POSITION // Trigger when train is at position moving in specific direction
};

class PositionAction : public SensorAction {
private:
    SensorLocation targetPosition;
    SensorLocation secondaryPosition; // Used for BETWEEN_POSITIONS condition
    PositionCondition condition;
    TrainDirection requiredDirection; // Used for DIRECTION_AT_POSITION condition
    std::unique_ptr<SensorAction> wrappedAction; // The actual action to execute
    bool hasExecuted; // Prevent multiple executions
    
public:
    // Constructor for AT_POSITION, AFTER_POSITION conditions
    PositionAction(SensorLocation position, PositionCondition cond, std::unique_ptr<SensorAction> action);
    
    // Constructor for BETWEEN_POSITIONS condition
    PositionAction(SensorLocation pos1, SensorLocation pos2, std::unique_ptr<SensorAction> action);
    
    // Constructor for DIRECTION_AT_POSITION condition
    PositionAction(SensorLocation position, TrainDirection direction, std::unique_ptr<SensorAction> action);
    
    void execute(TrainController& controller) override;
    void execute(TrainController& controller, ActionController& actionController) override;
    std::unique_ptr<SensorAction> clone() const override;
    
    // Position-specific methods
    bool checkCondition(const PositionTracker& positionTracker) const;
    void reset(); // Reset execution state
    
    // Getters
    SensorLocation getTargetPosition() const { return targetPosition; }
    PositionCondition getCondition() const { return condition; }
    bool hasBeenExecuted() const { return hasExecuted; }
};

#endif // POSITIONACTION_HPP
