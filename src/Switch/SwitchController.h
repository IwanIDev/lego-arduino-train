#ifndef SWITCH_CONTROLLER_H
#define SWITCH_CONTROLLER_H

#include <map>
#include <Arduino.h>

enum class SwitchPosition {
    STRAIGHT,
    DIVERGED
};

struct Switch {
    unsigned int id;
    unsigned int relayPin;
    SwitchPosition state;  // STRAIGHT = one direction, DIVERGED = other direction
};

class SwitchController {
private:
    std::map<unsigned int, Switch> switches;
    int nextAvailableId;
    void controlRelay(unsigned int relayPin, SwitchPosition state);
    
public:
    SwitchController();
    void setSwitchState(unsigned int switchId, SwitchPosition state);
    SwitchPosition getSwitchState(unsigned int switchId) const;
    void toggleSwitch(unsigned int switchId);
    int addSwitch(unsigned int id, unsigned int relayPin, SwitchPosition initialState = SwitchPosition::STRAIGHT);
    int addSwitch(unsigned int relayPin, SwitchPosition initialState = SwitchPosition::STRAIGHT);
    bool switchExists(unsigned int switchId) const;
    bool operateSwitch(unsigned int switchId, SwitchPosition position);
};

#endif // SWITCH_CONTROLLER_H