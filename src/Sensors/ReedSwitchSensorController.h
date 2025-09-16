#ifndef REED_SWITCH_SENSOR_CONTROLLER_H
#define REED_SWITCH_SENSOR_CONTROLLER_H
#define MAX_SENSORS 8
#include "ReedSwitchSensor.h"
#include <vector>

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
#endif // REED_SWITCH_SENSOR_CONTROLLER_H