#ifndef LIGHTSENSORCONTROLLER_HPP
#define LIGHTSENSORCONTROLLER_HPP
#define MAX_SENSORS 8
#include "LightSensor.hpp"

class LightSensorController {
private:
    LightSensor* sensors[MAX_SENSORS]; // Array of LightSensor pointers
    int sensorCount;
public:
    LightSensorController();

    bool isTrainPassingOver();

    void addSensor(LightSensor* sensor);
};
#endif // LIGHTSENSORCONTROLLER_HPP