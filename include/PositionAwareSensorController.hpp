#ifndef POSITIONAWARESENSORCONTROLLER_HPP
#define POSITIONAWARESENSORCONTROLLER_HPP

#include "ReedSwitchSensorController.hpp"
#include "LightSensorController.hpp"
#include "PositionTracker.hpp"

class PositionAwareSensorController {
private:
    ReedSwitchSensorController* reedSwitchController;
    LightSensorController* lightSensorController;
    SensorLocation lastTriggeredLocation;
    unsigned long lastTriggerTime;
    static const unsigned long DEBOUNCE_TIME = 100; // Reduced from 200ms to 100ms for faster response
    
public:
    PositionAwareSensorController(ReedSwitchSensorController* reedController, 
                                  LightSensorController* lightController);
    
    // Check sensors for triggers
    bool checkSensors();
    
    // Get the sensor that was triggered (for executing actions)
    Sensor* getTriggeredSensor() const;
    
    // Get triggered location
    SensorLocation getTriggeredLocation() const { return lastTriggeredLocation; }
    
    // Check if any sensor detected the train
    bool isTrainDetected() const;
};

#endif // POSITIONAWARESENSORCONTROLLER_HPP
