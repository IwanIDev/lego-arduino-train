#ifndef SENSOR_HPP
#define SENSOR_HPP

#include "TrainController.hpp"
#include "Action/SensorAction.hpp"
#include <memory>

class Sensor {
public:
    virtual ~Sensor() = default;
    virtual bool detectPassingTrain() = 0;
    virtual bool isTrainDetected() const = 0;
    virtual void reset() = 0;
    virtual void executeAction(TrainController& controller) = 0;
};

#endif // SENSOR_HPP