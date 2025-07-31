#ifndef SENSOR_ACTION_HPP
#define SENSOR_ACTION_HPP
#include "TrainController.hpp"
enum class SensorLocation {
    STATION_STOP,
    SPEED_REDUCE,
    DIRECTION_CHANGE,
    // Add more locations as needed
};

struct SensorAction {
    SPEED targetSpeed;
    bool changeDirection;
    unsigned long delayMs;
};

#endif // SENSOR_ACTION_HPP