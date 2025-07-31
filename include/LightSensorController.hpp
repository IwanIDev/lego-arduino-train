#ifndef LIGHTSENSORCONTROLLER_HPP
#define LIGHTSENSORCONTROLLER_HPP
#define MAX_SENSORS 8
#include "LightSensor.hpp"

class LightSensorController {
private:
    LightSensor* sensors[MAX_SENSORS]; // Array of LightSensor pointers
    int sensorCount;
    LightSensor* lastTriggeredSensor;

public:
    LightSensorController();

    bool isTrainPassingOver();

    SensorLocation getTriggeredSensorLocation() const;

    void addSensor(LightSensor* sensor);

    LightSensor* getTriggeredSensor() const;
};
#endif // LIGHTSENSORCONTROLLER_HPP