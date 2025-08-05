#ifndef REEDSWITCHSENSORCONTROLLER_HPP
#define REEDSWITCHSENSORCONTROLLER_HPP
#define MAX_SENSORS 8
#include "ReedSwitchSensor.hpp"

class ReedSwitchSensorController {
private:
    ReedSwitchSensor* sensors[MAX_SENSORS]; // Array of ReedSwitchSensor pointers
    int sensorCount;
    ReedSwitchSensor* lastTriggeredSensor;

public:
    ReedSwitchSensorController();

    bool isTrainPassingOver();

    SensorLocation getTriggeredSensorLocation() const;

    void addSensor(ReedSwitchSensor* sensor);

    ReedSwitchSensor* getTriggeredSensor() const;
};
#endif // REEDSWITCHSENSORCONTROLLER_HPP