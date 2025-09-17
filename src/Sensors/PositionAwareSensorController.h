#ifndef POSITION_AWARE_SENSOR_CONTROLLER_H
#define POSITION_AWARE_SENSOR_CONTROLLER_H

#include "ReedSwitchSensorController.h"
#include "LightSensorController.h"
#include "../Position/PositionTracker.h"
#include <vector>

class PositionAwareSensorController {
private:
    ReedSwitchSensorController* reedSwitchController;
    LightSensorController* lightSensorController;
    SensorLocation lastTriggeredLocation;
    unsigned long lastTriggerTime;
    static const unsigned long DEBOUNCE_TIME = 0;
    
public:
    PositionAwareSensorController(ReedSwitchSensorController* reedController, 
                                  LightSensorController* lightController);
    
    // Check sensors for triggers
    bool checkSensors();
    
    // Get the sensor that was triggered (for executing actions)
    std::vector<Sensor*> getTriggeredSensor() const;
    
    // Get triggered location
    SensorLocation getTriggeredLocation() const { return lastTriggeredLocation; }
    
    // Check if any sensor detected the train
    bool isTrainDetected() const;
};

#endif // POSITION_AWARE_SENSOR_CONTROLLER_H