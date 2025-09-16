#ifndef TRAIN_CONFIG_H
#define TRAIN_CONFIG_H

#include "../Actions/SensorAction.h"
#include "Arduino.h"

struct TrainConfig {
    String hubName;
    byte motorPort;
    int fastButtonPin;
    int slowButtonPin;
    SensorLocation initialPosition;
};

#endif // TRAIN_CONFIG_H