#ifndef REEDSWITCHSENSORCONTROLLER_HPP
#define REEDSWITCHSENSORCONTROLLER_HPP
#define MAX_SENSORS 8
#include "ReedSwitchSensor.hpp"

class ReedSwitchSensorController {
private:
    ReedSwitchSensor* sensors[MAX_SENSORS]; // Array of ReedSwitchSensor pointers
    int sensorCount;
    std::vector<ReedSwitchSensor*> lastTriggeredSensors;

public:
    ReedSwitchSensorController();

    bool isTrainPassingOver();

    SensorLocation getTriggeredSensorLocation() const;

    void addSensor(ReedSwitchSensor* sensor);

    std::vector<ReedSwitchSensor*> getTriggeredSensors() const;
};
#endif // REEDSWITCHSENSORCONTROLLER_HPP