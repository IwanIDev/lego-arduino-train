#ifndef SENSOR_ACTION_HPP
#define SENSOR_ACTION_HPP
#include "TrainController.hpp"

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
};
#endif // SENSOR_ACTION_HPP