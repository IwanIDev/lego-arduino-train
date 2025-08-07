#ifndef SENSOR_ACTION_HPP
#define SENSOR_ACTION_HPP
#include "TrainController.hpp"
#include <memory>

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
    virtual std::unique_ptr<SensorAction> clone() const = 0; // Virtual clone method
    virtual bool isDelayedAction() const { return false; } // Override in DelayedAction
};
#endif // SENSOR_ACTION_HPP