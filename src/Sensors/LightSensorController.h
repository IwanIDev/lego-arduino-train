#ifndef LIGHT_SENSOR_CONTROLLER_H
#define LIGHT_SENSOR_CONTROLLER_H
#define MAX_SENSORS 8
#include "LightSensor.h"

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
#endif // LIGHT_SENSOR_CONTROLLER_H