#ifndef SENSOR_ACTION_HPP
#define SENSOR_ACTION_HPP
#include "TrainController.hpp"
#include <memory>

// Forward declaration to avoid circular dependency
class ActionController;

enum class SensorLocation {
    STATION_STOP,
    SPEED_REDUCE,
    DIRECTION_CHANGE,
    // Add more locations as needed
};

class SensorAction {
public:
    virtual ~SensorAction() = default;
    virtual void execute(TrainController& controller) = 0;
    virtual void execute(TrainController& controller, ActionController& actionController);
    virtual std::unique_ptr<SensorAction> clone() const = 0; // Virtual clone method
    virtual bool isDelayedAction() const { return false; } // Override in DelayedAction
    virtual bool isSequentialAction() const { return false; } // Override in SequentialAction
};
#endif // SENSOR_ACTION_HPP