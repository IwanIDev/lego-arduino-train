#ifndef SENSOR_H
#define SENSOR_H

#include "../Controllers/TrainController.h"
#include "../Actions/SensorAction.h"
#include "../Position/SensorLocation.h"
#include <memory>

class ActionController; // Forward declaration

class Sensor {
public:
    virtual ~Sensor() = default;
    virtual bool detectPassingTrain() = 0;
    virtual bool isTrainDetected() const = 0;
    virtual void reset() = 0;
    virtual void executeAction(TrainController& controller, ActionController& actionController) = 0;
    virtual SensorLocation getLocation() const = 0;
};

#endif // SENSOR_H